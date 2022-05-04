/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_spmw_paths.h"
#include "RG.h"
#include "../value.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../datatypes/datatypes.h"
#include "../algorithms/all_paths.h"

#include <float.h>

// CALL algo.SPMWpaths()

typedef struct {
	Path *path;
	double weight;
	double cost;
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
	SIValue *out;                // result returned
	SIValue *yield_path;         // yield path
	SIValue *yield_path_weight;  // yield path weight
	SIValue *yield_path_cost;    // yield path cost
} SPMWctx;

static void _process_yield
(
	SPMWctx *ctx,
	const char **yield
) {
	ctx->yield_path         = NULL;
	ctx->yield_path_weight  = NULL;
	ctx->yield_path_cost    = NULL;

	int idx = 0;
	for(uint i = 0; i < array_len(yield); i++) {
		if(strcasecmp("path", yield[i]) == 0) {
			ctx->yield_path = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("pathWeight", yield[i]) == 0) {
			ctx->yield_path_weight = ctx->out + idx;
			idx++;
			continue;
		}

		if(strcasecmp("pathCost", yield[i]) == 0) {
			ctx->yield_path_cost = ctx->out + idx;
			idx++;
			continue;
		}
	}
}

ProcedureResult validate_config(SIValue config, SPMWctx *ctx) {
	SIValue start;
	SIValue end;
	SIValue relationships;
	SIValue dir;
	SIValue max_length;
	SIValue weight_prop;
	SIValue cost_prop;
	SIValue max_cost;
	SIValue path_count;
	
	bool start_exists         = MAP_GET(config, "sourceNode",   start);
	bool end_exists           = MAP_GET(config, "targetNode",   end);
	bool relationships_exists = MAP_GET(config, "relTypes",     relationships);
	bool dir_exists           = MAP_GET(config, "relDirection", dir);
	bool max_length_exists    = MAP_GET(config, "maxLen",       max_length);
	bool weight_prop_exists   = MAP_GET(config, "weightProp",   weight_prop);
	bool cost_prop_exists     = MAP_GET(config, "costProp",     cost_prop);
	bool max_cost_exists      = MAP_GET(config, "maxCost",      max_cost);
	bool path_count_exists    = MAP_GET(config, "pathCount",    path_count);
	

	if(!start_exists || !end_exists) {
		ErrorCtx_SetError("sourceNode and targetNode are required");
		return false;
	}
	if(SI_TYPE(start) != T_NODE || SI_TYPE(end) != T_NODE) {
		ErrorCtx_SetError("sourceNode and targetNode must be Node");
		return false;
	}
	
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Graph *g = QueryCtx_GetGraph();
	int *types = NULL;
	uint types_count = 0;
	if(relationships_exists) {
		if(SI_TYPE(relationships) != T_ARRAY) {
			ErrorCtx_SetError("relTypes must be array of strings");
			return false;
		}
		types_count = SIArray_Length(relationships);
		if(types_count > 0) {
			types = array_new(int, types_count);
			for (uint i = 0; i < types_count; i++) {
				SIValue rel = SIArray_Get(relationships, i);
				if(SI_TYPE(rel) != T_STRING) {
					ErrorCtx_SetError("relTypes must be array of strings");
					return false;
				}
				const char *type = rel.stringval;
				Schema *s = GraphContext_GetSchema(gc, type, SCHEMA_EDGE);
				array_append(types, Schema_GetID(s));
			}
		}
	}
	GRAPH_EDGE_DIR direction = GRAPH_EDGE_DIR_OUTGOING;
	if(dir_exists) {
		if(SI_TYPE(dir) != T_STRING) {
			ErrorCtx_SetError("relDirection must be a string");
			return false;
		}
		if(strcasecmp(dir.stringval, "incoming")) {
			direction = GRAPH_EDGE_DIR_INCOMING;
		} else if(strcasecmp(dir.stringval, "outgoing")) {
			direction = GRAPH_EDGE_DIR_OUTGOING;
		} else if(strcasecmp(dir.stringval, "both")) {
			direction = GRAPH_EDGE_DIR_BOTH;
		} else {
			ErrorCtx_SetError("relDirection values must be 'incoming', 'outgoing' or 'both'");
			return false;
		}
	}

	ctx->all_paths_ctx = AllPathsCtx_New((Node *)start.ptrval,
		(Node *)end.ptrval, g, types, types_count, direction, 1,
		max_length.longval, NULL, NULL, 0, false);

	if(weight_prop_exists) {
		if(SI_TYPE(weight_prop) != T_STRING) {
			ErrorCtx_SetError("weightProp must be a string");
			return false;
		}
		ctx->weight_prop = GraphContext_GetAttributeID(gc, weight_prop.stringval);
	} else {
		ctx->weight_prop = ATTRIBUTE_NOTFOUND;
	}

	if(cost_prop_exists) {
		if(SI_TYPE(cost_prop) != T_STRING) {
			ErrorCtx_SetError("costProp must be a string");
			return false;
		}
		ctx->cost_prop = GraphContext_GetAttributeID(gc, cost_prop.stringval);
	} else {
		ctx->cost_prop = ATTRIBUTE_NOTFOUND;
	}

	if(max_cost_exists) {
		if(SI_TYPE(max_cost) != T_INT64 && SI_TYPE(max_cost) != T_DOUBLE) {
			ErrorCtx_SetError("costProp must be numeric");
			return false;
		}
		ctx->max_cost = SI_GET_NUMERIC(max_cost);
	}

	if(path_count_exists) {
		if(SI_TYPE(path_count) != T_INT64) {
				ErrorCtx_SetError("pathCount must be integer");
				return false;
		}
		ctx->path_count = path_count.longval;
	} else {
		ctx->path_count = 1;
	}

	return true;
}

// Check to see if context levels array has entries at position 'level'.
static bool _AllPathsCtx_LevelNotEmpty(const AllPathsCtx *ctx, uint level) {
	return (level < array_len(ctx->levels) && array_len(ctx->levels[level]) > 0);
}

void SPMWpaths_next
(
	SPMWctx *ctx,
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
				SIValue c = ctx->cost_prop == ATTRIBUTE_NOTFOUND
					? SI_LongVal(1)
					: *GraphEntity_GetProperty((GraphEntity *)&frontierConnection.edge, ctx->cost_prop);
				SIValue w = ctx->weight_prop == ATTRIBUTE_NOTFOUND
					? SI_LongVal(1)
					: *GraphEntity_GetProperty((GraphEntity *)&frontierConnection.edge, ctx->weight_prop);
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
				if(allPathsCtx->dst != NULL) {
					Node dst = Path_Head(allPathsCtx->path);
					if(ENTITY_GET_ID(allPathsCtx->dst) != ENTITY_GET_ID(&dst)) continue;
				}
				p->path = allPathsCtx->path;
				return;
			}
		} else {
			// No way to advance, backtrack.
			Path_PopNode(allPathsCtx->path);
			if(Path_EdgeCount(allPathsCtx->path)) {
				Edge e = Path_PopEdge(allPathsCtx->path);
				SIValue c = ctx->cost_prop == ATTRIBUTE_NOTFOUND
					? SI_LongVal(1)
					: *GraphEntity_GetProperty((GraphEntity *)&e, ctx->cost_prop);
				p->cost -= SI_GET_NUMERIC(c);
				SIValue w = ctx->weight_prop == ATTRIBUTE_NOTFOUND
					? SI_LongVal(1)
					: *GraphEntity_GetProperty((GraphEntity *)&e, ctx->weight_prop);
				p->weight -= SI_GET_NUMERIC(w);
			}
		}
	}

	// Couldn't find a path.
	p->path = NULL;
	return;
}

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

void SPMWpaths_all_minimal
(
	SPMWctx *spmw_ctx
) {
	spmw_ctx->array = array_new(WeightedPath, 0);

	WeightedPath p = {0};
	double max_weight = DBL_MAX;
	SPMWpaths_next(spmw_ctx, &p, max_weight);
	while (p.path != NULL) {
		uint count = array_len(spmw_ctx->array);
		if(count > 0 && p.weight < spmw_ctx->array[0].weight) {
			for (uint i = 0; i < array_len(spmw_ctx->array); i++) {
				Path_Free(spmw_ctx->array[i].path);
			}
			array_clear(spmw_ctx->array);
			count = 0;
		}
		p.path = Path_Clone(p.path);
		array_append(spmw_ctx->array, p);
		max_weight = p.weight;
		SPMWpaths_next(spmw_ctx, &p, max_weight);
	}
}

void SPMWpaths_single_minimal
(
	SPMWctx *spmw_ctx
) {
	spmw_ctx->single.path   = NULL;
	spmw_ctx->single.weight = DBL_MAX;
	spmw_ctx->single.cost   = DBL_MAX;
	WeightedPath p = {0};
	SPMWpaths_next(spmw_ctx, &p, DBL_MAX);
	while (p.path != NULL) {
		if(p.weight < spmw_ctx->single.weight ||
			(p.weight == spmw_ctx->single.weight &&
				(p.cost < spmw_ctx->single.cost || Path_Len(p.path) < Path_Len(spmw_ctx->single.path)))) {
			spmw_ctx->single.path = Path_Clone(p.path);
			spmw_ctx->single.weight = p.weight;
			spmw_ctx->single.cost = p.cost;
		}
		SPMWpaths_next(spmw_ctx, &p, spmw_ctx->single.weight);
	}
}

void SPMWpaths_k_minimal
(
	SPMWctx *spmw_ctx
) {
	spmw_ctx->heap = Heap_new(path_cmp, NULL);

	WeightedPath p = {0};
	double max_weight = DBL_MAX;
	SPMWpaths_next(spmw_ctx, &p, max_weight);
	while (p.path != NULL) {
		if(Heap_count(spmw_ctx->heap) == spmw_ctx->path_count) {
			WeightedPath *pp = Heap_peek(spmw_ctx->heap);
			if(p.weight < pp->weight) {
				Heap_poll(spmw_ctx->heap);
				Path_Free(pp->path);
				pp->path = Path_Clone(p.path);
				pp->weight = p.weight;
				pp->cost = p.cost;
				Heap_offer(&spmw_ctx->heap, pp);
			}
			max_weight = pp->weight;
		} else {
			WeightedPath *pp = rm_malloc(sizeof(WeightedPath));
			pp->path = Path_Clone(p.path);
			pp->weight = p.weight;
			pp->cost = p.cost;
			Heap_offer(&spmw_ctx->heap, pp);
		}
		SPMWpaths_next(spmw_ctx, &p, max_weight);
	}
}

ProcedureResult Proc_SPMWpathsInvoke
(
	ProcedureCtx *ctx,
	const SIValue *args,
	const char **yield
) {
	SPMWctx *spmw_ctx = rm_malloc(sizeof(SPMWctx));
	ctx->privateData = spmw_ctx;
	if(!validate_config(args[0], ctx->privateData)) return PROCEDURE_ERR;

	spmw_ctx->out = array_new(SIValue, 3);
	_process_yield(spmw_ctx, yield);

	if(spmw_ctx->path_count == 0) SPMWpaths_all_minimal(spmw_ctx);
	else if(spmw_ctx->path_count == 1) SPMWpaths_single_minimal(spmw_ctx);
	else SPMWpaths_k_minimal(spmw_ctx);

	return PROCEDURE_OK;
}

SIValue *Proc_SPMWpathsStep
(
	ProcedureCtx *ctx
) {
	ASSERT(ctx->privateData != NULL);
	
	SPMWctx *spmw_ctx = ctx->privateData;
	WeightedPath p;

	if(spmw_ctx->path_count == 0) {
		if(array_len(spmw_ctx->array) == 0) return NULL;

		p = array_pop(spmw_ctx->array);
	} else if(spmw_ctx->path_count == 1) {
		p = spmw_ctx->single;
		if(p.path == NULL) return NULL;

		spmw_ctx->single.path = NULL;
	} else {
		WeightedPath *pp = Heap_poll(spmw_ctx->heap);
		if(pp == NULL) return NULL;
		
		p = *pp;
		rm_free(pp);
	}
	
	if(spmw_ctx->yield_path)        *spmw_ctx->yield_path        = SI_Path(p.path);
	if(spmw_ctx->yield_path_weight) *spmw_ctx->yield_path_weight = SI_DoubleVal(p.weight);
	if(spmw_ctx->yield_path_cost)   *spmw_ctx->yield_path_cost   = SI_DoubleVal(p.cost);

	return spmw_ctx->out;
}

ProcedureResult Proc_SPMWpathsFree
(
	ProcedureCtx *ctx
) {
	SPMWctx *spmw_ctx = ctx->privateData;
	if(spmw_ctx != NULL) {
		AllPathsCtx_Free(spmw_ctx->all_paths_ctx);
		if(spmw_ctx->path_count == 0) array_free(spmw_ctx->array);
		else if(spmw_ctx->path_count > 1) Heap_free(spmw_ctx->heap);
		array_free(spmw_ctx->out);
		rm_free(spmw_ctx);
	}
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_SPMWpathCtx() {
	void *privateData = NULL;
	ProcedureOutput output;
	ProcedureOutput *outputs = array_new(ProcedureOutput, 3);
	output = (ProcedureOutput){.name = "path", .type = T_PATH | T_NULL};
	array_append(outputs, output);
	output = (ProcedureOutput){.name = "pathWeight", .type = T_DOUBLE | T_NULL};
	array_append(outputs, output);
	output = (ProcedureOutput){.name = "pathCost", .type = T_DOUBLE | T_NULL};
	array_append(outputs, output);

	ProcedureCtx *ctx = ProcCtxNew("algo.SPMWpaths",
								   1,
								   outputs,
								   Proc_SPMWpathsStep,
								   Proc_SPMWpathsInvoke,
								   Proc_SPMWpathsFree,
								   privateData,
								   true);
	return ctx;
}
