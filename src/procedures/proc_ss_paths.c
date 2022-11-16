/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "proc_ss_paths.h"
#include "../value.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/datatypes.h"

#include <float.h>

// MATCH (n:L {v: 1})
// CALL algo.SSpaths({sourceNode: n,
//					  relTypes: ['E'],
//					  maxLen: 3,
//					  weightProp: 'weight',
//					  costProp: 'cost',
//					  maxCost: 4,
//					  pathCount: 1}) YIELD path, pathWeight, pathCost
// RETURN path, pathWeight, pathCost

typedef struct {
	Path *path;      // path
	double weight;   // path weight
	double cost;     // path cost
} WeightedPath;

typedef struct {
	Node node;
	Edge edge;
} LevelConnection;

typedef struct {
	LevelConnection **levels;    // nodes reached at depth i, and edges leading to them.
	Path *path;                  // current path.
	Graph *g;                    // graph to traverse.
	Edge *neighbors;             // reusable buffer of edges along the current path.
	int *relationIDs;            // edge type(s) to traverse.
	int relationCount;           // length of relationIDs.
	GRAPH_EDGE_DIR dir;          // traverse direction.
	uint minLen;                 // path minimum length.
	uint maxLen;                 // path max length.
	Attribute_ID weight_prop;    // weight attribute id
	Attribute_ID cost_prop;      // cost attribuite id
	double max_cost;             // maximum cost of path
	uint64_t path_count;         // path to return
	union {
		WeightedPath single;     // path_count == 1
		heap_t *heap;            // in case path_count > 1
		WeightedPath *array;     // path_count == 0 return all minimum result
	};                           // path collection
	SIValue *output;             // result returned
	SIValue *yield_path;         // yield path
	SIValue *yield_path_weight;  // yield path weight
	SIValue *yield_path_cost;    // yield path cost
} SingleSourceCtx;

// free SingleSourceCtx
static void SingleSourceCtx_Free
(
	SingleSourceCtx *ctx
) {
	if(ctx == NULL) return;

	uint32_t levelsCount = array_len(ctx->levels);
	for(int i = 0; i < levelsCount; i++) array_free(ctx->levels[i]);
	if(ctx->levels) array_free(ctx->levels);
	if(ctx->path) Path_Free(ctx->path);
	if(ctx->neighbors) array_free(ctx->neighbors);
	if(ctx->relationIDs) {
		array_free(ctx->relationIDs);
	}
	if(ctx->path_count == 0 && ctx->array != NULL) array_free(ctx->array);
	else if(ctx->path_count > 1 && ctx->heap != NULL) Heap_free(ctx->heap);
	array_free(ctx->output);
	rm_free(ctx);
}

// initialize returned values pointers
static void _process_yield
(
	SingleSourceCtx *ctx,
	const char **yield
) {
	ctx->yield_path         = NULL;
	ctx->yield_path_weight  = NULL;
	ctx->yield_path_cost    = NULL;

	int idx = 0;
	for(uint i = 0; i < array_len(yield); i++) {
		if(strcasecmp("path", yield[i]) == 0) {
			ctx->yield_path = ctx->output + idx;
			idx++;
			continue;
		}

		if(strcasecmp("pathWeight", yield[i]) == 0) {
			ctx->yield_path_weight = ctx->output + idx;
			idx++;
			continue;
		}

		if(strcasecmp("pathCost", yield[i]) == 0) {
			ctx->yield_path_cost = ctx->output + idx;
			idx++;
			continue;
		}
	}
}

// make sure context level array have 'cap' available entries.
static void _SingleSourceCtx_EnsureLevelArrayCap
(
	SingleSourceCtx *ctx,
	uint level,
	uint cap
) {
	uint len = array_len(ctx->levels);
	if(level < len) {
		LevelConnection *current = ctx->levels[level];
		ctx->levels[level] = array_ensure_cap(current, array_len(current) + cap);
		return;
	}

	ASSERT(level == len);
	array_append(ctx->levels, array_new(LevelConnection, cap));
}

// append given 'node' to given 'level' array.
static void _SingleSourceCtx_AddConnectionToLevel
(
	SingleSourceCtx *ctx,
	uint level,
	Node *node,
	Edge *edge
) {
	ASSERT(level < array_len(ctx->levels));
	LevelConnection connection;
	connection.node = *node;
	if(edge) connection.edge = *edge;
	array_append(ctx->levels[level], connection);
}

static void SingleSourceCtx_New
(
	SingleSourceCtx *ctx,
	Node *src,
	Graph *g,
	int *relationIDs,
	int relationCount,
	GRAPH_EDGE_DIR dir,
	uint minLen,
	uint maxLen
) {
	ASSERT(src != NULL);

	ctx->g              =  g;
	ctx->dir            =  dir;
	ctx->minLen         =  minLen + 1;
	ctx->maxLen         =  maxLen + 1;
	ctx->relationIDs    =  relationIDs;
	ctx->relationCount  =  relationCount;
	ctx->levels         =  array_new(LevelConnection *, 1);
	ctx->path           =  Path_New(1);
	ctx->neighbors      =  array_new(Edge, 32);

	_SingleSourceCtx_EnsureLevelArrayCap(ctx, 0, 1);
	_SingleSourceCtx_AddConnectionToLevel(ctx, 0, src, NULL);
}


// validate config map and initialize SingleSourceCtx
static ProcedureResult validate_config
(
	SIValue config,
	SingleSourceCtx *ctx
) {
	SIValue start;                // start node
	SIValue relationships;        // relationship types allowed
	SIValue dir;                  // direction
	SIValue max_length;           // max traverse length
	SIValue weight_prop;          // weight attribute name
	SIValue cost_prop;            // cost attribute name
	SIValue max_cost;             // maximum cost
	SIValue path_count;           // # of paths to return
	
	bool start_exists         = MAP_GET(config, "sourceNode",   start);
	bool relationships_exists = MAP_GET(config, "relTypes",     relationships);
	bool dir_exists           = MAP_GET(config, "relDirection", dir);
	bool max_length_exists    = MAP_GET(config, "maxLen",       max_length);
	bool weight_prop_exists   = MAP_GET(config, "weightProp",   weight_prop);
	bool cost_prop_exists     = MAP_GET(config, "costProp",     cost_prop);
	bool max_cost_exists      = MAP_GET(config, "maxCost",      max_cost);
	bool path_count_exists    = MAP_GET(config, "pathCount",    path_count);
	

	if(!start_exists) {
		ErrorCtx_SetError("sourceNode is required");
		return false;
	}
	if(SI_TYPE(start) != T_NODE) {
		ErrorCtx_SetError("sourceNode must be of type Node");
		return false;
	}

	GRAPH_EDGE_DIR direction = GRAPH_EDGE_DIR_OUTGOING;
	if(dir_exists) {
		if(SI_TYPE(dir) != T_STRING) {
			ErrorCtx_SetError("relDirection values must be 'incoming', 'outgoing' or 'both'");
			return false;
		}
		if(strcasecmp(dir.stringval, "incoming") == 0) {
			direction = GRAPH_EDGE_DIR_INCOMING;
		} else if(strcasecmp(dir.stringval, "outgoing") == 0) {
			direction = GRAPH_EDGE_DIR_OUTGOING;
		} else if(strcasecmp(dir.stringval, "both") == 0) {
			direction = GRAPH_EDGE_DIR_BOTH;
		} else {
			ErrorCtx_SetError("relDirection values must be 'incoming', 'outgoing' or 'both'");
			return false;
		}
	}

	int64_t max_length_val = LONG_MAX - 1;
	if(max_length_exists) {
		if(SI_TYPE(max_length) != T_INT64) {
			ErrorCtx_SetError("maxLen must be integer");
			return false;
		}
		max_length_val = SI_GET_NUMERIC(max_length);
	}

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = QueryCtx_GetGraph();
	int *types = NULL;
	uint types_count = 0;
	if(relationships_exists) {
		if(SI_TYPE(relationships) != T_ARRAY ||
			!SIArray_AllOfType(relationships, T_STRING)) {
			ErrorCtx_SetError("relTypes must be array of strings");
			return false;
		}
		types_count = SIArray_Length(relationships);
		if(types_count > 0) {
			types = array_new(int, types_count);
			for (uint i = 0; i < types_count; i++) {
				SIValue rel = SIArray_Get(relationships, i);
				const char *type = rel.stringval;
				Schema *s = GraphContext_GetSchema(gc, type, SCHEMA_EDGE);
				if(s == NULL) continue;
				array_append(types, Schema_GetID(s));
			}
			types_count = array_len(types);
		}
	} else {
		types_count = 1;
		types = array_new(int, types_count);
		array_append(types, GRAPH_NO_RELATION);
	}

	SingleSourceCtx_New(ctx, (Node *)start.ptrval, g, types, types_count,
		direction, 1, max_length_val);

	ctx->weight_prop = ATTRIBUTE_ID_NONE;
	ctx->cost_prop = ATTRIBUTE_ID_NONE;
	ctx->max_cost = DBL_MAX;
	ctx->path_count = 1;

	if(weight_prop_exists) {
		if(SI_TYPE(weight_prop) != T_STRING) {
			ErrorCtx_SetError("weightProp must be a string");
			return false;
		}
		ctx->weight_prop = GraphContext_GetAttributeID(gc, weight_prop.stringval);
	}

	if(cost_prop_exists) {
		if(SI_TYPE(cost_prop) != T_STRING) {
			ErrorCtx_SetError("costProp must be a string");
			return false;
		}
		ctx->cost_prop = GraphContext_GetAttributeID(gc, cost_prop.stringval);
	}

	if(max_cost_exists) {
		if(SI_TYPE(max_cost) != T_INT64 && SI_TYPE(max_cost) != T_DOUBLE) {
			ErrorCtx_SetError("maxCost must be numeric");
			return false;
		}
		ctx->max_cost = SI_GET_NUMERIC(max_cost);
	}

	if(path_count_exists) {
		if(SI_TYPE(path_count) != T_INT64) {
				ErrorCtx_SetError("pathCount must be integer");
				return false;
		}
		if(path_count.longval < 0) {
				ErrorCtx_SetError("pathCount must be greater than or equal to 0");
				return false;
		}
		ctx->path_count = SI_GET_NUMERIC(path_count);
	}

	return true;
}

// check to see if context levels array has entries at position 'level'.
static bool _SingleSourceCtx_LevelNotEmpty
(
	const SingleSourceCtx *ctx,
	uint level
) {
	return (level < array_len(ctx->levels) && array_len(ctx->levels[level]) > 0);
}

static void addOutgoingNeighbors
(
	SingleSourceCtx *ctx,
	LevelConnection *frontier,
	uint32_t depth
) {
	EntityID frontierId = INVALID_ENTITY_ID;
	if(depth > 1) frontierId = ENTITY_GET_ID(&frontier->edge);

	// Get frontier neighbors.
	for(int i = 0; i < ctx->relationCount; i++) {
		Graph_GetNodeEdges(ctx->g, &frontier->node, GRAPH_EDGE_DIR_OUTGOING, ctx->relationIDs[i], &ctx->neighbors);
	}

	// Add unvisited neighbors to next level.
	uint32_t neighborsCount = array_len(ctx->neighbors);

	_SingleSourceCtx_EnsureLevelArrayCap(ctx, depth, neighborsCount);
	for(uint32_t i = 0; i < neighborsCount; i++) {
		// Don't follow the frontier edge again.
		if(frontierId == ENTITY_GET_ID(ctx->neighbors + i)) continue;
		// Set the neighbor by following the edge in the correct directoin.
		Node neighbor = GE_NEW_NODE();
		Graph_GetNode(ctx->g, Edge_GetDestNodeID(ctx->neighbors + i), &neighbor);
		// Add the node and edge to the frontier.
		_SingleSourceCtx_AddConnectionToLevel(ctx, depth, &neighbor, (ctx->neighbors + i));
	}
	array_clear(ctx->neighbors);
}

static void addIncomingNeighbors
(
	SingleSourceCtx *ctx,
	LevelConnection *frontier,
	uint32_t depth
) {
	EntityID frontierId = INVALID_ENTITY_ID;
	if(depth > 1) frontierId = ENTITY_GET_ID(&frontier->edge);

	// Get frontier neighbors.
	for(int i = 0; i < ctx->relationCount; i++) {
		Graph_GetNodeEdges(ctx->g, &frontier->node, GRAPH_EDGE_DIR_INCOMING, ctx->relationIDs[i], &ctx->neighbors);
	}

	// Add unvisited neighbors to next level.
	uint32_t neighborsCount = array_len(ctx->neighbors);

	_SingleSourceCtx_EnsureLevelArrayCap(ctx, depth, neighborsCount);
	for(uint32_t i = 0; i < neighborsCount; i++) {
		// Don't follow the frontier edge again.
		if(frontierId == ENTITY_GET_ID(ctx->neighbors + i)) continue;
		// Set the neighbor by following the edge in the correct directoin.
		Node neighbor = GE_NEW_NODE();
		Graph_GetNode(ctx->g, Edge_GetSrcNodeID(ctx->neighbors + i), &neighbor);
		// Add the node and edge to the frontier.
		_SingleSourceCtx_AddConnectionToLevel(ctx, depth, &neighbor, (ctx->neighbors + i));
	}
	array_clear(ctx->neighbors);
}

// traverse from the frontier node in the specified direction and add all encountered nodes and edges.
static void addNeighbors
(
	SingleSourceCtx *ctx,
	LevelConnection *frontier,
	uint32_t depth,
	GRAPH_EDGE_DIR dir
) {
	switch(dir) {
		case GRAPH_EDGE_DIR_OUTGOING:
			addOutgoingNeighbors(ctx, frontier, depth);
			break;
		case GRAPH_EDGE_DIR_INCOMING:
			addIncomingNeighbors(ctx, frontier, depth);
			break;
		case GRAPH_EDGE_DIR_BOTH:
			addIncomingNeighbors(ctx, frontier, depth);
			addOutgoingNeighbors(ctx, frontier, depth);
			break;
		default:
			ASSERT(false && "encountered unexpected traversal direction in AllPaths");
			break;
	}
}

// get numeric attribute value of an entity otherwise return default value
static inline SIValue _get_value_or_defualt
(
	GraphEntity *ge,
	Attribute_ID id,
	SIValue default_value
) {
	SIValue *v = GraphEntity_GetProperty(ge, id);
	if(v == ATTRIBUTE_NOTFOUND) return default_value;

	if(SI_TYPE(*v) & SI_NUMERIC) return *v;

	return default_value;
}

// use DFS to find all paths from src tracking cost and weight
static void SSpaths_next
(
	SingleSourceCtx *ctx,
	WeightedPath *p,
	double max_weight
) {
	// as long as path is not empty OR there are neighbors to traverse.
	while(Path_NodeCount(ctx->path) || _SingleSourceCtx_LevelNotEmpty(ctx, 0)) {
		uint32_t depth = Path_NodeCount(ctx->path);

		// can we advance?
		if(_SingleSourceCtx_LevelNotEmpty(ctx, depth)) {
			// get a new frontier.
			LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
			Node frontierNode = frontierConnection.node;

			bool frontierAlreadyOnPath = Path_ContainsNode(ctx->path, &frontierNode);

			// don't allow cycles
			if(frontierAlreadyOnPath) continue;

			// add frontier to path.
			Path_AppendNode(ctx->path, frontierNode);

			// if depth is 0 this is the source node, there is no leading edge to it.
			// for depth > 0 for each frontier node, there is a leading edge.
			if(depth > 0) {
				SIValue c = _get_value_or_defualt((GraphEntity *)&frontierConnection.edge, ctx->cost_prop, SI_LongVal(1));
				SIValue w = _get_value_or_defualt((GraphEntity *)&frontierConnection.edge, ctx->weight_prop, SI_LongVal(1));
				if(p->cost + SI_GET_NUMERIC(c) <= ctx->max_cost && p->weight + SI_GET_NUMERIC(w) <= max_weight) {
					p->cost += SI_GET_NUMERIC(c);
					p->weight += SI_GET_NUMERIC(w);
					Path_AppendEdge(ctx->path, frontierConnection.edge);
				} else {
					Path_PopNode(ctx->path);
					continue;
				}
			}

			// update path depth.
			depth++;

			// introduce neighbors only if path depth < maximum path length.
			// and frontier wasn't already expanded.
			if(depth < ctx->maxLen) {
				addNeighbors(ctx, &frontierConnection, depth, ctx->dir);
			}

			// see if we can return path.
			if(depth >= ctx->minLen && depth <= ctx->maxLen) {
				p->path = ctx->path;
				return;
			}
		} else {
			// no way to advance, backtrack.
			Path_PopNode(ctx->path);
			if(Path_EdgeCount(ctx->path)) {
				Edge e = Path_PopEdge(ctx->path);
				SIValue c = _get_value_or_defualt((GraphEntity *)&e, ctx->cost_prop, SI_LongVal(1));
				SIValue w = _get_value_or_defualt((GraphEntity *)&e, ctx->weight_prop, SI_LongVal(1));
				p->cost -= SI_GET_NUMERIC(c);
				p->weight -= SI_GET_NUMERIC(w);
			}
		}
	}

	// couldn't find a path.
	p->path = NULL;
	return;
}

// compare path by weight, cost and path length
static int path_cmp
(
	const void *a,
	const void *b,
	void *udata
) {
	WeightedPath *da = (WeightedPath *)a;
	WeightedPath *db = (WeightedPath *)b;
	if(da->weight == db->weight) {
		if(da->cost == db->cost) {
			return Path_Len(da->path) - Path_Len(db->path);
		}
		return da->cost - db->cost;
	}
	return da->weight - db->weight;
}

// get all minimal paths (all paths with the same weight)
static void SSpaths_all_minimal
(
	SingleSourceCtx *ctx
) {
	// initialize array that contains the result
	ctx->array = array_new(WeightedPath, 0);

	// get first path
	WeightedPath p = {0};
	double max_weight = DBL_MAX;
	SSpaths_next(ctx, &p, max_weight);

	// iterate over all paths
	while (p.path != NULL) {
		// if current path is better and the array is not empty clear it
		uint count = array_len(ctx->array);
		if(count > 0 && p.weight < ctx->array[0].weight) {
			for (uint i = 0; i < array_len(ctx->array); i++) {
				Path_Free(ctx->array[i].path);
			}
			array_clear(ctx->array);
		}

		// add the path to the result array
		p.path = Path_Clone(p.path);
		array_append(ctx->array, p);

		// update max weight
		max_weight = p.weight;

		// get next path where path weight is <= max_weight
		SSpaths_next(ctx, &p, max_weight);
	}
}

// find the single minimal weighted path
static void SSpaths_single_minimal
(
	SingleSourceCtx *ctx
) {
	// initialize the result path to worst path
	ctx->single.path   = NULL;
	ctx->single.weight = DBL_MAX;
	ctx->single.cost   = DBL_MAX;

	// get first path
	WeightedPath p = {0};
	SSpaths_next(ctx, &p, DBL_MAX);

	// iterate over all paths
	while (p.path != NULL) {
		// if the current path is better replace it
		if(p.weight < ctx->single.weight ||
			p.cost < ctx->single.cost ||
			(p.cost == ctx->single.cost &&
				Path_Len(p.path) < Path_Len(ctx->single.path))) {
			if(ctx->single.path != NULL) {
				Path_Free(ctx->single.path);
			}
			ctx->single.path = Path_Clone(p.path);
			ctx->single.weight = p.weight;
			ctx->single.cost = p.cost;
		}

		// get next path where path weight is <= result weight
		SSpaths_next(ctx, &p, ctx->single.weight);
	}
}

static void inline _add_path
(
	heap_t **heap,
	WeightedPath *p
) {
	WeightedPath *pp = rm_malloc(sizeof(WeightedPath));
	pp->path = Path_Clone(p->path);
	pp->weight = p->weight;
	pp->cost = p->cost;
	Heap_offer(heap, pp);
}

// find k minimal weighted path (path can have different weight)
static void SSpaths_k_minimal
(
	SingleSourceCtx *ctx
) {
	// initialize heap that contains the result where top path is the highest weight
	ctx->heap = Heap_new(path_cmp, NULL);

	// get first path
	WeightedPath p = {0};
	double max_weight = DBL_MAX;
	SSpaths_next(ctx, &p, max_weight);

	// iterate over all paths
	while (p.path != NULL && Heap_count(ctx->heap) < ctx->path_count - 1) {
		// fill the heap
		_add_path(&ctx->heap, &p);

		// get next path where path weight is <= max_weight
		SSpaths_next(ctx, &p, max_weight);
	}

	if(p.path == NULL) return;

	// fill the heap
	_add_path(&ctx->heap, &p);

	// update the max weight so we will get better paths
	WeightedPath *pp = Heap_peek(ctx->heap);
	max_weight = pp->weight;

	// get next path where path weight is <= max_weight
	SSpaths_next(ctx, &p, max_weight);

	while (p.path != NULL) {
		// if the heap is full check if the current path is better 
		// than the worst path if yes replace it
		pp = Heap_peek(ctx->heap);
		if(p.weight < pp->weight ||
			p.cost < pp->cost ||
			(p.cost == pp->cost &&
				Path_Len(p.path) < Path_Len(pp->path))) {
			Heap_poll(ctx->heap);
			Path_Free(pp->path);
			pp->path = Path_Clone(p.path);
			pp->weight = p.weight;
			pp->cost = p.cost;
			Heap_offer(&ctx->heap, pp);

			// update the max weight so we will get better paths
			pp = Heap_peek(ctx->heap);
			max_weight = pp->weight;
		}

		// get next path where path weight is <= max_weight
		SSpaths_next(ctx, &p, max_weight);
	}
}

static ProcedureResult Proc_SSpathsInvoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {
	SingleSourceCtx *single_source_ctx = rm_calloc(1, sizeof(SingleSourceCtx));
	if(!validate_config(args[0], single_source_ctx)) {
		SingleSourceCtx_Free(single_source_ctx);
		return PROCEDURE_ERR;
	}
	ctx->privateData = single_source_ctx;

	single_source_ctx->output = array_new(SIValue, 3);
	_process_yield(single_source_ctx, yield);

	if(single_source_ctx->path_count == 0) SSpaths_all_minimal(single_source_ctx);
	else if(single_source_ctx->path_count == 1) SSpaths_single_minimal(single_source_ctx);
	else SSpaths_k_minimal(single_source_ctx);

	return PROCEDURE_OK;
}

static SIValue *Proc_SSpathsStep
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData != NULL);
	
	SingleSourceCtx *single_source_ctx = ctx->privateData;
	WeightedPath p;

	if(single_source_ctx->path_count == 0) {
		if(array_len(single_source_ctx->array) == 0) return NULL;

		p = array_pop(single_source_ctx->array);
	} else if(single_source_ctx->path_count == 1) {
		p = single_source_ctx->single;
		if(p.path == NULL) return NULL;

		single_source_ctx->single.path = NULL;
	} else {
		WeightedPath *pp = Heap_poll(single_source_ctx->heap);
		if(pp == NULL) return NULL;
		
		p = *pp;
		rm_free(pp);
	}
	
	if(single_source_ctx->yield_path) {
		*single_source_ctx->yield_path = SI_Path(p.path);
		Path_Free(p.path);
	}
	if(single_source_ctx->yield_path_weight) *single_source_ctx->yield_path_weight = SI_DoubleVal(p.weight);
	if(single_source_ctx->yield_path_cost)   *single_source_ctx->yield_path_cost   = SI_DoubleVal(p.cost);

	return single_source_ctx->output;
}

static ProcedureResult Proc_SSpathsFree
(
	ProcedureCtx *ctx
) {
	SingleSourceCtx *single_source_ctx = ctx->privateData;
	SingleSourceCtx_Free(single_source_ctx);
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_SSpathCtx() {
	void *privateData = NULL;
	ProcedureOutput output;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 3);
	output = (ProcedureOutput){.name = "path", .type = T_PATH | T_NULL};
	array_append(outputs, output);
	output = (ProcedureOutput){.name = "pathWeight", .type = T_DOUBLE | T_NULL};
	array_append(outputs, output);
	output = (ProcedureOutput){.name = "pathCost", .type = T_DOUBLE | T_NULL};
	array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("algo.SSpaths",
								   1,
								   outputs,
								   Proc_SSpathsStep,
								   Proc_SSpathsInvoke,
								   Proc_SSpathsFree,
								   privateData,
								   true);
	return ctx;
}
