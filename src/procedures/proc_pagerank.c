/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_pagerank.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../algorithms/pagerank.h"

// CALL algo.pageRank('Page', 'LINKS', {iterations: 20, dampingFactor: 0.85}) YIELD node, score
// CALL algo.pageRank('Page', 'LINKS') YIELD node, score

typedef struct {
	int n;                          // Number of nodes to rank.
	int i;                          // Current node to return.
	Graph *g;                       // Graph.
	Node node;                      // Node.
	GrB_Index *mappings;            // Mappings between extracted matrix rows and node ids.
	LAGraph_PageRank *rankings;     // Nodes rankings.
	SIValue *output;                // Array with 4 entries ["node", node, "score", score].
} PagerankContext;

ProcedureResult Proc_PagerankInvoke(ProcedureCtx *ctx, const char **args) {
	if(array_len(args) != 2) return PROCEDURE_ERR;
	const char *label = args[0];
	const char *relation = args[1];

	GrB_Index n = 0;
	Schema *s = NULL;
	GrB_Matrix l = NULL;
	GrB_Matrix r = NULL;
	GrB_Index *mappings = NULL; // Mappings, array for returning row indices of tuples.
	GrB_Matrix reduced = GrB_NULL;
	Graph *g = QueryCtx_GetGraph();
	LAGraph_PageRank *rankings = NULL;
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Setup context.
	PagerankContext *pdata = rm_malloc(sizeof(PagerankContext));
	pdata->n = n;
	pdata->i = 0;
	pdata->g = g;
	pdata->mappings = mappings;
	pdata->rankings = rankings;
	pdata->output = array_new(SIValue, 4);
	pdata->output = array_append(pdata->output, SI_ConstStringVal("node"));
	pdata->output = array_append(pdata->output, SI_Node(NULL)); // Place holder.
	pdata->output = array_append(pdata->output, SI_ConstStringVal("score"));
	pdata->output = array_append(pdata->output, SI_DoubleVal(0.0)); // Place holder.
	ctx->privateData = pdata;

	// Get label matrix.
	s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
	if(!s) return PROCEDURE_OK;
	l = Graph_GetLabelMatrix(g, s->id);

	// Get relation matrix.
	s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
	if(!s) return PROCEDURE_OK;
	r = Graph_GetRelationMatrix(g, s->id);

	GrB_Index rows = Graph_RequiredMatrixDim(g);
	GrB_Index cols = rows;
	assert(GrB_Matrix_nvals(&n, l) == GrB_SUCCESS);
	assert(GrB_Matrix_new(&reduced, GrB_BOOL, n, n) == GrB_SUCCESS);

	if(n != rows) {
		mappings = rm_malloc(sizeof(GrB_Index) * n);
		assert(GrB_Matrix_extractTuples_BOOL(mappings, GrB_NULL, GrB_NULL, &n, l) == GrB_SUCCESS);
		assert(GrB_extract(reduced, GrB_NULL, GrB_NULL, r, mappings, n, mappings, n,
						   GrB_NULL) == GrB_SUCCESS);
	} else {
		/* There no need to perform extraction as `r` dimension NxN
		 * is the same as the number of entries in `l` which means
		 * all connections described in `r` connect nodes of type `l`
		 * Unfortunately we still need to type cast `r` into a boolean matrix. */
		GrB_Descriptor desc;
		GrB_Descriptor_new(&desc);
		GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		assert(GrB_transpose(reduced, GrB_NULL, GrB_NULL, r, desc) == GrB_SUCCESS);
		GrB_free(&desc);
	}


	double tol = 1e-4;
	int iters, itermax = 100;
	assert(Pagerank(&rankings, reduced, itermax, tol, &iters) == GrB_SUCCESS);

	// Clean up.
	GrB_free(&reduced);

	// Update context.
	pdata->n = n;
	pdata->mappings = mappings;
	pdata->rankings = rankings;
	return PROCEDURE_OK;
}

SIValue *Proc_PagerankStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);

	PagerankContext *pdata = (PagerankContext *)ctx->privateData;

	// Depleted?
	if(pdata->i >= pdata->n) return NULL;

	LAGraph_PageRank rank = pdata->rankings[pdata->i++];
	NodeID node_id = (pdata->mappings) ? pdata->mappings[rank.page] : rank.page;

	Graph_GetNode(pdata->g, node_id, &pdata->node);
	pdata->output[1] = SI_Node(&pdata->node);
	pdata->output[3] = SI_DoubleVal(rank.pagerank);

	return pdata->output;
}

ProcedureResult Proc_PagerankFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) {
		PagerankContext *pdata = ctx->privateData;
		if(pdata->output) array_free(pdata->output);
		if(pdata->mappings) rm_free(pdata->mappings);
		if(pdata->rankings) rm_free(pdata->rankings);
		rm_free(ctx->privateData);
	}

	return PROCEDURE_OK;
}

ProcedureCtx *Proc_PagerankCtx() {
	void *privateData = NULL;
	ProcedureOutput **outputs = array_new(ProcedureOutput *, 2);
	ProcedureOutput *output_node = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_score = rm_malloc(sizeof(ProcedureOutput));
	output_node->name = "node";
	output_node->type = T_NODE;
	output_score->name = "score";
	output_score->type = T_DOUBLE;

	outputs = array_append(outputs, output_node);
	outputs = array_append(outputs, output_score);
	ProcedureCtx *ctx = ProcCtxNew("db.pageRank",
								   2,
								   outputs,
								   Proc_PagerankStep,
								   Proc_PagerankInvoke,
								   Proc_PagerankFree,
								   privateData,
								   false);
	return ctx;
}
