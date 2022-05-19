/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "proc_ssmw_paths.h"
#include "../value.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/datatypes.h"
#include "../algorithms/all_paths.h"

#include <float.h>

// MATCH (n:L {v: 1})
// CALL algo.SSMinWpaths({sourceNode: n,
//						  relTypes: ['E'],
//						  maxLen: 3,
//						  weightProp: 'weight',
//						  costProp: 'cost',
//						  maxCost: 4,
//						  pathCount: 1}) YIELD path, pathWeight, pathCost
// RETURN path, pathWeight, pathCost

typedef struct {
	Path *path;      // path
	double weight;   // path weight
	double cost;     // path cost
} WeightedPath;

typedef struct {
	AllPathsCtx *all_paths_ctx;  // path traverse details
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
} SSMWctx;

// free SSMWctx
static void SSMWctx_Free
(
	SSMWctx *ctx
) {
	if(ctx == NULL) return;

	if(ctx->all_paths_ctx && ctx->all_paths_ctx->relationIDs) {
		array_free(ctx->all_paths_ctx->relationIDs);
	}
	AllPathsCtx_Free(ctx->all_paths_ctx);
	if(ctx->path_count == 0 && ctx->array != NULL) array_free(ctx->array);
	else if(ctx->path_count > 1 && ctx->heap != NULL) Heap_free(ctx->heap);
	array_free(ctx->output);
	rm_free(ctx);
}

// initialize returned values pointers
static void _process_yield
(
	SSMWctx *ctx,
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

// validate config map and initialize SSMWctx
static ProcedureResult validate_config(SIValue config, SSMWctx *ctx) {
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

	int64_t max_length_val = LONG_MAX;
	if(max_length_exists) {
		if(SI_TYPE(max_length) != T_INT64) {
			ErrorCtx_SetError("max_length must be integer");
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
	}

	ctx->all_paths_ctx = AllPathsCtx_New((Node *)start.ptrval,
		NULL, g, types, types_count, direction, 1,
		max_length_val, NULL, NULL, 0, false);

	ctx->weight_prop = ATTRIBUTE_NOTFOUND;
	ctx->cost_prop = ATTRIBUTE_NOTFOUND;
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
		ctx->path_count = SI_GET_NUMERIC(path_count);
	}

	return true;
}

// Check to see if context levels array has entries at position 'level'.
static bool _AllPathsCtx_LevelNotEmpty(const AllPathsCtx *ctx, uint level) {
	return (level < array_len(ctx->levels) && array_len(ctx->levels[level]) > 0);
}

// get numeric attribute value of an entity otherwise return default value
static inline SIValue _get_value_or_defualt
(
	GraphEntity *ge,
	Attribute_ID id,
	SIValue default_value
) {
	SIValue *v = GraphEntity_GetProperty(ge, id);
	if(v == PROPERTY_NOTFOUND) return default_value;

	if(SI_TYPE(*v) & SI_NUMERIC) return *v;

	return default_value;
}

// use DFS to find all paths from src tracking cost and weight
static void SSMWpaths_next
(
	SSMWctx *ctx,
	WeightedPath *p,
	double max_weight
) {
	AllPathsCtx *allPathsCtx = ctx->all_paths_ctx;

	// As long as path is not empty OR there are neighbors to traverse.
	while(Path_NodeCount(allPathsCtx->path) || _AllPathsCtx_LevelNotEmpty(allPathsCtx, 0)) {
		uint32_t depth = Path_NodeCount(allPathsCtx->path);

		// Can we advance?
		if(_AllPathsCtx_LevelNotEmpty(allPathsCtx, depth)) {
			// Get a new frontier.
			LevelConnection frontierConnection = array_pop(allPathsCtx->levels[depth]);
			Node frontierNode = frontierConnection.node;

			/* See if frontier is already on path,
			 * it is OK for a path to contain an entity twice,
			 * such as in the case of a cycle, but in such case we
			 * won't expand frontier.
			 * i.e. closing a cycle and continuing traversal. */
			bool frontierAlreadyOnPath = Path_ContainsNode(allPathsCtx->path, &frontierNode);

			// Add frontier to path.
			Path_AppendNode(allPathsCtx->path, frontierNode);

			/* If depth is 0 this is the source node, there is no leading edge to it.
			 * For depth > 0 for each frontier node, there is a leading edge. */
			if(depth > 0) {
				SIValue c = _get_value_or_defualt((GraphEntity *)&frontierConnection.edge, ctx->cost_prop, SI_LongVal(1));
				SIValue w = _get_value_or_defualt((GraphEntity *)&frontierConnection.edge, ctx->weight_prop, SI_LongVal(1));
				if(p->cost + SI_GET_NUMERIC(c) <= ctx->max_cost && p->weight + SI_GET_NUMERIC(w) <= max_weight) {
					p->cost += SI_GET_NUMERIC(c);
					p->weight += SI_GET_NUMERIC(w);
					Path_AppendEdge(allPathsCtx->path, frontierConnection.edge);
				} else {
					Path_PopNode(allPathsCtx->path);
					continue;
				}
			}

			// Update path depth.
			depth++;

			/* Introduce neighbors only if path depth < maximum path length.
			 * and frontier wasn't already expanded. */
			if(depth < allPathsCtx->maxLen && !frontierAlreadyOnPath) {
				addNeighbors(allPathsCtx, &frontierConnection, depth, allPathsCtx->dir);
			}

			// See if we can return path.
			if(depth >= allPathsCtx->minLen && depth <= allPathsCtx->maxLen) {
				p->path = allPathsCtx->path;
				return;
			}
		} else {
			// No way to advance, backtrack.
			Path_PopNode(allPathsCtx->path);
			if(Path_EdgeCount(allPathsCtx->path)) {
				Edge e = Path_PopEdge(allPathsCtx->path);
				SIValue c = _get_value_or_defualt((GraphEntity *)&e, ctx->cost_prop, SI_LongVal(1));
				SIValue w = _get_value_or_defualt((GraphEntity *)&e, ctx->weight_prop, SI_LongVal(1));
				p->cost -= SI_GET_NUMERIC(c);
				p->weight -= SI_GET_NUMERIC(w);
			}
		}
	}

	// Couldn't find a path.
	p->path = NULL;
	return;
}

// compare path by weight, cost and path length
static int path_cmp(const void *a, const void *b, const void *udata) {
	WeightedPath *da = (WeightedPath *)a;
	WeightedPath *db = (WeightedPath *)b;
	if(da->weight == db->weight) {
		if(da->cost - db->cost) {
			return Path_Len(da->path) - Path_Len(db->path);
		}
		return da->cost - db->cost;
	}
	return da->weight - db->weight;
}

// get all minimal paths (all paths with the same weight)
static void SSMWpaths_all_minimal
(
	SSMWctx *ssmw_ctx
) {
	// initialize array that contains the result
	ssmw_ctx->array = array_new(WeightedPath, 0);

	// get first path
	WeightedPath p = {0};
	double max_weight = DBL_MAX;
	SSMWpaths_next(ssmw_ctx, &p, max_weight);

	// iterate over all paths
	while (p.path != NULL) {
		// if current path is better and the array is not empty clear it
		uint count = array_len(ssmw_ctx->array);
		if(count > 0 && p.weight < ssmw_ctx->array[0].weight) {
			for (uint i = 0; i < array_len(ssmw_ctx->array); i++) {
				Path_Free(ssmw_ctx->array[i].path);
			}
			array_clear(ssmw_ctx->array);
		}

		// add the path to the result array
		p.path = Path_Clone(p.path);
		array_append(ssmw_ctx->array, p);

		// update max weight
		max_weight = p.weight;

		// get next path where path weight is <= max_weight
		SSMWpaths_next(ssmw_ctx, &p, max_weight);
	}
}

// find the single minimal weighted path
static void SSMWpaths_single_minimal
(
	SSMWctx *ssmw_ctx
) {
	// initialize the result path to worst path
	ssmw_ctx->single.path   = NULL;
	ssmw_ctx->single.weight = DBL_MAX;
	ssmw_ctx->single.cost   = DBL_MAX;

	// get first path
	WeightedPath p = {0};
	SSMWpaths_next(ssmw_ctx, &p, DBL_MAX);

	// iterate over all paths
	while (p.path != NULL) {
		// if the current path is better replace it
		if(p.weight < ssmw_ctx->single.weight ||
			p.cost < ssmw_ctx->single.cost ||
			(p.cost == ssmw_ctx->single.cost &&
				Path_Len(p.path) < Path_Len(ssmw_ctx->single.path))) {
			if(ssmw_ctx->single.path != NULL) {
				Path_Free(ssmw_ctx->single.path);
			}
			ssmw_ctx->single.path = Path_Clone(p.path);
			ssmw_ctx->single.weight = p.weight;
			ssmw_ctx->single.cost = p.cost;
		}

		// get next path where path weight is <= result weight
		SSMWpaths_next(ssmw_ctx, &p, ssmw_ctx->single.weight);
	}
}

// find k minimal weighted path (path can have different weight)
static void SSMWpaths_k_minimal
(
	SSMWctx *ssmw_ctx
) {
	// initialize heap that contains the result where top path is the highest weight
	ssmw_ctx->heap = Heap_new(path_cmp, NULL);

	// get first path
	WeightedPath p = {0};
	double max_weight = DBL_MAX;
	SSMWpaths_next(ssmw_ctx, &p, max_weight);

	// iterate over all paths
	while (p.path != NULL) {
		if(Heap_count(ssmw_ctx->heap) == ssmw_ctx->path_count) {
			// if the heap is full check if the current path is better 
			// than the worst path if yes replace it
			WeightedPath *pp = Heap_peek(ssmw_ctx->heap);
			if(p.weight < pp->weight ||
				p.cost < pp->cost ||
				(p.cost == pp->cost &&
					Path_Len(p.path) < Path_Len(pp->path))) {
				Heap_poll(ssmw_ctx->heap);
				Path_Free(pp->path);
				pp->path = Path_Clone(p.path);
				pp->weight = p.weight;
				pp->cost = p.cost;
				Heap_offer(&ssmw_ctx->heap, pp);
			}

			// update the max weight so we will get better paths
			pp = Heap_peek(ssmw_ctx->heap);
			max_weight = pp->weight;
		} else {
			// fill the heap
			WeightedPath *pp = rm_malloc(sizeof(WeightedPath));
			pp->path = Path_Clone(p.path);
			pp->weight = p.weight;
			pp->cost = p.cost;
			Heap_offer(&ssmw_ctx->heap, pp);
		}

		// get next path where path weight is <= max_weight
		SSMWpaths_next(ssmw_ctx, &p, max_weight);
	}
}

static ProcedureResult Proc_SSMWpathsInvoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {
	SSMWctx *ssmw_ctx = rm_calloc(1, sizeof(SSMWctx));
	if(!validate_config(args[0], ssmw_ctx)) {
		SSMWctx_Free(ssmw_ctx);
		return PROCEDURE_ERR;
	}
	ctx->privateData = ssmw_ctx;

	ssmw_ctx->output = array_new(SIValue, 3);
	_process_yield(ssmw_ctx, yield);

	if(ssmw_ctx->path_count == 0) SSMWpaths_all_minimal(ssmw_ctx);
	else if(ssmw_ctx->path_count == 1) SSMWpaths_single_minimal(ssmw_ctx);
	else SSMWpaths_k_minimal(ssmw_ctx);

	return PROCEDURE_OK;
}

static SIValue *Proc_SSMWpathsStep
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData != NULL);
	
	SSMWctx *ssmw_ctx = ctx->privateData;
	WeightedPath p;

	if(ssmw_ctx->path_count == 0) {
		if(array_len(ssmw_ctx->array) == 0) return NULL;

		p = array_pop(ssmw_ctx->array);
	} else if(ssmw_ctx->path_count == 1) {
		p = ssmw_ctx->single;
		if(p.path == NULL) return NULL;

		ssmw_ctx->single.path = NULL;
	} else {
		WeightedPath *pp = Heap_poll(ssmw_ctx->heap);
		if(pp == NULL) return NULL;
		
		p = *pp;
		rm_free(pp);
	}
	
	if(ssmw_ctx->yield_path)        *ssmw_ctx->yield_path        = SI_Path(p.path);
	if(ssmw_ctx->yield_path_weight) *ssmw_ctx->yield_path_weight = SI_DoubleVal(p.weight);
	if(ssmw_ctx->yield_path_cost)   *ssmw_ctx->yield_path_cost   = SI_DoubleVal(p.cost);

	return ssmw_ctx->output;
}

static ProcedureResult Proc_SSMWpathsFree
(
	ProcedureCtx *ctx
) {
	SSMWctx *ssmw_ctx = ctx->privateData;
	SSMWctx_Free(ssmw_ctx);
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_SSMWpathCtx() {
	void *privateData = NULL;
	ProcedureOutput output;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 3);
	output = (ProcedureOutput){.name = "path", .type = T_PATH | T_NULL};
	array_append(outputs, output);
	output = (ProcedureOutput){.name = "pathWeight", .type = T_DOUBLE | T_NULL};
	array_append(outputs, output);
	output = (ProcedureOutput){.name = "pathCost", .type = T_DOUBLE | T_NULL};
	array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("algo.SSMinWpaths",
								   1,
								   outputs,
								   Proc_SSMWpathsStep,
								   Proc_SSMWpathsInvoke,
								   Proc_SSMWpathsFree,
								   privateData,
								   true);
	return ctx;
}
