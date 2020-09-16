/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_pagerank.h"
#include "../RG.h"
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
	GrB_Index *mapping;             // Mapping between extracted matrix rows and node ids.
	LAGraph_PageRank *ranking;      // Nodes ranking.
	SIValue *output;                // Array with 4 entries ["node", node, "score", score].
} PagerankContext;

ProcedureResult Proc_PagerankInvoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 2) return PROCEDURE_ERR;
	if(!(SI_TYPE(args[0]) & SI_TYPE(args[1]) & T_STRING)) return PROCEDURE_ERR;

	const char *label = args[0].stringval;
	const char *relation = args[1].stringval;

	GrB_Info info;
	GrB_Index n = 0;               // Node count
	GrB_Index nvals;               // Number of entries in 'reduced'
	GrB_Index nrows;               // Relation matrix row count
	Schema *s = NULL;
	GrB_Matrix l = NULL;           // Label matrix
	GrB_Matrix r = NULL;           // Relation matrix
	GrB_Index *mapping = NULL;     // Mapping, array for returning row indices of tuples
	GrB_Matrix reduced = GrB_NULL; // Relation matrix reduced to only 'l' rows
	Graph *g = QueryCtx_GetGraph();
	LAGraph_PageRank *ranking = NULL;
	GraphContext *gc = QueryCtx_GetGraphCtx();

	// Setup context.
	PagerankContext *pdata = rm_malloc(sizeof(PagerankContext));
	pdata->n = n;
	pdata->i = 0;
	pdata->g = g;
	pdata->node = GE_NEW_NODE();
	pdata->mapping = mapping;
	pdata->ranking = ranking;
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

	//--------------------------------------------------------------------------
	// Create a NxN matrix, one row for each labeled entity
	//--------------------------------------------------------------------------
	info = GrB_Matrix_nvals(&n, l);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_nrows(&nrows, r);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_new(&reduced, GrB_BOOL, n, n);
	ASSERT(info == GrB_SUCCESS);

	// Discard rows of 'r' associated with nodes of a different type than 'l'
	if(n != nrows) {
		mapping = rm_malloc(sizeof(GrB_Index) * n);
		// Extract row indecies from 'l', coresponding to node IDs.
		info = GrB_Matrix_extractTuples_BOOL(mapping, GrB_NULL, GrB_NULL, &n, l);
		ASSERT(info == GrB_SUCCESS);

		info = GrB_Matrix_extract(reduced, GrB_NULL, GrB_NULL, r, mapping, n,
			mapping, n, GrB_NULL);
		ASSERT(info == GrB_SUCCESS);
	} else {
		/* There no need to perform extraction as `r` dimension NxN
		 * is the same as the number of entries in `l` which means
		 * all connections described in `r` connect nodes of type `l`
		 * Unfortunately we still need to type cast `r` into a boolean matrix. */
		GrB_Descriptor desc;
		GrB_Descriptor_new(&desc);
		GrB_Descriptor_set(desc, GrB_INP0, GrB_TRAN);
		info = GrB_transpose(reduced, GrB_NULL, GrB_NULL, r, desc);
		ASSERT(info	== GrB_SUCCESS);
		GrB_free(&desc);
	}

	// Invoke Pagerank only if 'reduced' contains entries.
	info = GrB_Matrix_nvals(&nvals, reduced);
	ASSERT(info == GrB_SUCCESS);

	if(nvals > 0) {
		double tol = 1e-4;
		int iters, itermax = 100;
		info = Pagerank(&ranking, reduced, itermax, tol, &iters);
		ASSERT(info == GrB_SUCCESS);
	}

	// Clean up.
	GrB_free(&reduced);

	// Update context.
	pdata->n = n;
	pdata->mapping = mapping;
	pdata->ranking = ranking;
	return PROCEDURE_OK;
}

SIValue *Proc_PagerankStep(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData);

	PagerankContext *pdata = (PagerankContext *)ctx->privateData;

	// Depleted/no results
	if(pdata->i >= pdata->n || pdata->ranking == NULL) return NULL;

	LAGraph_PageRank rank = pdata->ranking[pdata->i++];
	NodeID node_id = (pdata->mapping) ? pdata->mapping[rank.page] : rank.page;

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
		if(pdata->mapping) rm_free(pdata->mapping);
		if(pdata->ranking) rm_free(pdata->ranking);
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
	ProcedureCtx *ctx = ProcCtxNew("algo.pageRank",
								   2,
								   outputs,
								   Proc_PagerankStep,
								   Proc_PagerankInvoke,
								   Proc_PagerankFree,
								   privateData,
								   true);
	return ctx;
}

