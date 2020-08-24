/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_pagerank.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../config.h"
#include "../algorithms/LAGraph_bfs_both.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

// CALL algo.pageRank('Page', 'LINKS', {iterations: 20, dampingFactor: 0.85}) YIELD node, score
// CALL algo.pageRank('Page', 'LINKS') YIELD node, score

typedef struct {
	GrB_Index n;                    // Total number of results.
	GrB_Index i;                    // Current index into output arrays.
	Graph *g;                       // Graph.
	GrB_Index *node_ids;
	int64_t *node_levels;
	Node node;                      // Node.
	GrB_Vector V;                   // Output vector.
	GrB_Vector PI;                  // Parent vector.
	SIValue *output;                // Array with 4 entries ["node", node, "score", score].
} BFSContext;

ProcedureResult Proc_BFS_Invoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 4) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_NODE                ||
	   SI_TYPE(args[1]) != T_INT64               ||
	   !(SI_TYPE(args[2]) & (T_NULL | T_STRING)) ||
	   SI_TYPE(args[3]) != T_BOOL)
		return PROCEDURE_ERR;

	Node *source_node = args[0].ptrval;
	GrB_Index source_id = ENTITY_GET_ID(source_node);
	int64_t max_level = args[1].longval;
	const char *reltype = SIValue_IsNull(args[2]) ? NULL : args[2].stringval;
	bool track_parents = args[3].longval;

	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Setup context.
	BFSContext *pdata = rm_malloc(sizeof(BFSContext));
	pdata->i = 0;
	pdata->g = gc->g;
	pdata->node = GE_NEW_NODE();
	pdata->output = array_new(SIValue, 6);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
	pdata->output = array_append(pdata->output, SI_Node(NULL)); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("level"));
	pdata->output = array_append(pdata->output, SI_LongVal(0)); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("parent"));
	pdata->output = array_append(pdata->output, SI_Node(NULL)); // Place holder.
	ctx->privateData = pdata;

	// Get edge matrix and transpose matrix, if available.
	GrB_Matrix R;
	GrB_Matrix TR;
	if(reltype == NULL) {
		R = Graph_GetAdjacencyMatrix(gc->g);
		TR = Graph_GetTransposedAdjacencyMatrix(gc->g);
	} else {
		Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
		if(!s) return PROCEDURE_OK;
		R = Graph_GetRelationMatrix(gc->g, s->id);
		if(Config_MaintainTranspose()) TR = Graph_GetTransposedRelationMatrix(gc->g, s->id);
		else TR = GrB_NULL;
	}

	GrB_Vector V;
	GrB_Vector PI;
	if(track_parents == false) PI = GrB_NULL;

	assert(LAGraph_bfs_both(&V, &PI, R, TR, source_id, max_level, false, NULL) == GrB_SUCCESS);

	// Update context.
	pdata->V = V;
	pdata->PI = PI;

	// Get number of entries.
	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, V);
	pdata->n = nvals;

	// Retrieve all tuples.
	GrB_Index *node_ids = rm_malloc(nvals * sizeof(GrB_Index));
	int64_t *node_levels = rm_malloc(nvals * sizeof(GrB_Index));
	GrB_Vector_extractTuples_INT64(node_ids, node_levels, &nvals, V);
	pdata->node_ids = node_ids;
	pdata->node_levels = node_levels;
	if(track_parents) {

	}
	return PROCEDURE_OK;
}

SIValue *Proc_BFS_Step(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	BFSContext *pdata = (BFSContext *)ctx->privateData;

	// Depleted?
	if(pdata->i >= pdata->n) return NULL;

	NodeID node_id = pdata->node_ids[pdata->i];

	Graph_GetNode(pdata->g, node_id, &pdata->node);
	pdata->output[1] = SI_Node(&pdata->node);
	pdata->output[3] = SI_LongVal(pdata->node_levels[pdata->i]);

	pdata->i++;
	return pdata->output;
}

ProcedureResult Proc_BFS_Free(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		BFSContext *pdata = ctx->privateData;
		if(pdata->output) array_free(pdata->output);
		if(pdata->node_ids) rm_free(pdata->node_ids);
		if(pdata->node_levels) rm_free(pdata->node_levels);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_BFS_Ctx() {
	void *privateData = NULL;
	ProcedureOutput **outputs = array_new(ProcedureOutput *, 3);
	ProcedureOutput *output_node = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_level = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_parent = rm_malloc(sizeof(ProcedureOutput));
	output_node->name = "node";
	output_node->type = T_NODE;
	output_level->name = "level";
	output_level->type = T_INT64;
	output_parent->name = "parent";
	output_parent->type = T_NODE;

	outputs = array_append(outputs, output_node);
	outputs = array_append(outputs, output_parent);
	ProcedureCtx *ctx = ProcCtxNew("algo.BFS",
								   3,
								   outputs,
								   Proc_BFS_Step,
								   Proc_BFS_Invoke,
								   Proc_BFS_Free,
								   privateData,
								   true);
	return ctx;
}

