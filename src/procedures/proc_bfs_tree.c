/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_bfs.h"
#include "../value.h"
#include "../config.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../algorithms/LAGraph_bfs_both.h"

// MATCH (a:User {id: 1}) CALL algo.bfs(a, 0, 'MANAGES') YIELD node, level, path

typedef struct {
	Graph *g;                       // Graph.
	GrB_Index n;                    // Total number of results.
	GrB_Index i;                    // Current index into output arrays.
	GrB_Index *node_ids;            // Array of connected nodes.
	int64_t *node_levels;           // Level of each node in node_ids.
	GrB_Vector parents;             // Vector associating each node in the BFS tree with its parent.
	Node node;                      // Node.
	Node parent;                    // Parent node.
	SIValue *output;                // Array with 6 entries ["node", node, "level", level, "path", path].
} BFSContext;

static ProcedureResult Proc_BFSTree_Invoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 3) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_NODE                ||   // Source node.
	   SI_TYPE(args[1]) != T_INT64               ||   // Max level to iterate to, unlimited if 0.
	   !(SI_TYPE(args[2]) & (T_NULL | T_STRING)))     // Relationship type to traverse if not NULL.
		return PROCEDURE_ERR;

	Node *source_node = args[0].ptrval;
	GrB_Index source_id = ENTITY_GET_ID(source_node);
	int64_t max_level = args[1].longval;
	const char *reltype = SIValue_IsNull(args[2]) ? NULL : args[2].stringval;

	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Setup context.
	BFSContext *pdata = rm_malloc(sizeof(BFSContext));
	pdata->i = 0;
	pdata->n = 0;
	pdata->g = gc->g;
	pdata->node = GE_NEW_NODE();
	pdata->parent = GE_NEW_NODE();
	pdata->output = array_new(SIValue, 6);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
	pdata->output = array_append(pdata->output, SI_Node(NULL)); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("level"));
	pdata->output = array_append(pdata->output, SI_LongVal(0)); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("path"));
	pdata->output = array_append(pdata->output, SI_NullVal()); // Place holder.
	ctx->privateData = pdata;

	// Get edge matrix and transpose matrix, if available.
	GrB_Matrix R;
	GrB_Matrix TR;
	if(reltype == NULL) {
		R = Graph_GetAdjacencyMatrix(gc->g);
		TR = Graph_GetTransposedAdjacencyMatrix(gc->g);
	} else {
		Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
		if(!s) return PROCEDURE_OK; // Failed to find schema, first step will return NULL.
		R = Graph_GetRelationMatrix(gc->g, s->id);
		if(Config_MaintainTranspose()) TR = Graph_GetTransposedRelationMatrix(gc->g, s->id);
		else TR = GrB_NULL;
	}

	GrB_Vector V;  // Vector of results
	GrB_Vector PI; // Vector backtracking results to their parents.
	assert(LAGraph_bfs_both(&V, &PI, R, TR, source_id, max_level, false) == GrB_SUCCESS);

	// Remove all values with a level of 0, as they are not connected to the source.
	GxB_Vector_select(V, GrB_NULL, GrB_NULL, GxB_NONZERO, V, GrB_NULL, GrB_NULL);

	// Get number of entries.
	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, V);
	pdata->n = nvals;

	// Retrieve all tuples representing connected nodes and their levels.
	GrB_Index *node_ids = rm_malloc(nvals * sizeof(GrB_Index));
	int64_t *node_levels = rm_malloc(nvals * sizeof(GrB_Index));
	GrB_Vector_extractTuples_INT64(node_ids, node_levels, &nvals, V);
	pdata->node_ids = node_ids;
	pdata->node_levels = node_levels;
	pdata->parents = PI;

	GrB_Vector_free(&V);
	return PROCEDURE_OK;
}

static SIValue *Proc_BFSTree_Step(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	BFSContext *pdata = (BFSContext *)ctx->privateData;

	// Depleted?
	if(pdata->i >= pdata->n) return NULL;

	// Get the reached node.
	NodeID node_id = pdata->node_ids[pdata->i];
	Graph_GetNode(pdata->g, node_id, &pdata->node);

	int level = pdata->node_levels[pdata->i];
	pdata->i++; // Increment the node index.

	// Construct the path from dest->src.
	Node path[level];
	path[0] = pdata->node;
	for(int i = 1; i < level; i ++) {
		GrB_Index parent_id;
		assert(GrB_Vector_extractElement(&parent_id, pdata->parents, node_id) == GrB_SUCCESS);
		Graph_GetNode(pdata->g, parent_id - 1, &pdata->parent);
		path[i] = pdata->parent;
		node_id = parent_id - 1;
	}
	// Reverse the path (building src->dest) and store as an SIValue.
	SIValue path_val = SI_Array(level);
	for(int i = level - 1; i >= 0; i--) {
		SIArray_Append(&path_val, SI_Node(&path[i]));
	}

	pdata->output[1] = SI_Node(&pdata->node);
	pdata->output[3] = SI_LongVal(level);
	pdata->output[5] = path_val;

	return pdata->output;
}

static ProcedureResult Proc_BFSTree_Free(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		BFSContext *pdata = ctx->privateData;
		if(pdata->output) array_free(pdata->output);
		if(pdata->node_ids) rm_free(pdata->node_ids);
		if(pdata->parents) GrB_Vector_free(&pdata->parents);
		if(pdata->node_levels) rm_free(pdata->node_levels);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_BFSTree_Ctx() {
	void *privateData = NULL;
	ProcedureOutput **outputs = array_new(ProcedureOutput *, 3);
	ProcedureOutput *output_node = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_level = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_parent = rm_malloc(sizeof(ProcedureOutput));
	output_node->name = "node";
	output_node->type = T_NODE;
	output_level->name = "level";
	output_level->type = T_INT64;
	output_parent->name = "path";
	output_parent->type = T_ARRAY;

	outputs = array_append(outputs, output_node);
	outputs = array_append(outputs, output_level);
	outputs = array_append(outputs, output_parent);
	ProcedureCtx *ctx = ProcCtxNew("algo.BFSTree",
								   3,
								   outputs,
								   Proc_BFSTree_Step,
								   Proc_BFSTree_Invoke,
								   Proc_BFSTree_Free,
								   privateData,
								   true);
	return ctx;
}

