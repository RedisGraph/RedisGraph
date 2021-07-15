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

// CALL algo.pageRank(NULL, NULL)      YIELD node, score
// CALL algo.pageRank('Page', NULL)    YIELD node, score
// CALL algo.pageRank(NULL, 'LINKS')   YIELD node, score
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

ProcedureResult Proc_PagerankInvoke(ProcedureCtx *ctx,
									const SIValue *args, const char **yield) {
//	// Expecting 2 arguments.
//	if(array_len((SIValue *)args) != 2) return PROCEDURE_ERR;
//	// arg0 and arg1 can be either String or NULL
//	SIType arg0_t = SI_TYPE(args[0]);
//	SIType arg1_t = SI_TYPE(args[1]);
//	if(!(arg0_t & (T_STRING | T_NULL))) return PROCEDURE_ERR;
//	if(!(arg1_t & (T_STRING | T_NULL))) return PROCEDURE_ERR;
//
//	// Read arguments.
//	const char *label = NULL;    // Node filter.
//	const char *relation = NULL; // Edge filter.
//	if(arg0_t == T_STRING) label = args[0].stringval;
//	if(arg1_t == T_STRING) relation = args[1].stringval;
//
//	// Pagerank config arguments
//	int iters;         // iterations performed
//	const double tol = 1e-4; // tolerance
//	const int itermax = 100; // max iterations
//
//	GrB_Info info;
//	UNUSED(info);
//	GrB_Index n = 0;               // Node count
//	GrB_Index nvals;               // Number of entries in 'r'
//	GrB_Index nrows;               // Relation matrix row count
//	Schema *s = NULL;
//	GrB_Matrix l = NULL;           // Label matrix
//	GrB_Matrix r = NULL;           // Relation matrix
//	GrB_Index *mapping = NULL;     // Mapping, array for returning row indices of tuples
//	Graph *g = QueryCtx_GetGraph();
//	LAGraph_PageRank *ranking = NULL;
//	GraphContext *gc = QueryCtx_GetGraphCtx();
//
//	// Setup context.
//	PagerankContext *pdata = rm_malloc(sizeof(PagerankContext));
//	pdata->n = n;
//	pdata->i = 0;
//	pdata->g = g;
//	pdata->node = GE_NEW_NODE();
//	pdata->mapping = mapping;
//	pdata->ranking = ranking;
//	pdata->output = array_new(SIValue, 4);
//	array_append(pdata->output, SI_ConstStringVal("node"));
//	array_append(pdata->output, SI_Node(NULL)); // Place holder.
//	array_append(pdata->output, SI_ConstStringVal("score"));
//	array_append(pdata->output, SI_DoubleVal(0.0)); // Place holder.
//	ctx->privateData = pdata;
//
//	// Get label matrix.
//	if(label) {
//		s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
//		// Unknown label, quickly return.
//		if(!s) return PROCEDURE_OK;
//		l = Graph_GetLabelMatrix(g, s->id);
//	}
//
//	// Get relation matrix.
//	if(relation) {
//		s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
//		// Unknown relation, quickly return.
//		if(!s) return PROCEDURE_OK;
//		r = Graph_GetRelationMatrix(g, s->id);
//	} else {
//		// Relation isn't specified, 'r' is the adjacency matrix.
//		r = Graph_GetAdjacencyMatrix(g);
//	}
//	/* if label is specified:
//	 * filter 'r' to contain only rows and columns associated with
//	 * nodes of type 'l' */
//	if(label != NULL) {
//		//----------------------------------------------------------------------
//		// Create a NxN matrix, one row for each labeled entity
//		//----------------------------------------------------------------------
//		info = GrB_Matrix_nvals(&n, l);
//		ASSERT(info == GrB_SUCCESS);
//
//		GrB_Matrix reduced; // Relation matrix reduced to only 'l' rows/cols
//		info = GrB_Matrix_new(&reduced, GrB_BOOL, n, n);
//		ASSERT(info == GrB_SUCCESS);
//
//		// Discard rows of 'r' associated with nodes of a different type than 'l'
//		// this will also perform casting to boolean.
//		mapping = rm_malloc(sizeof(GrB_Index) * n);
//		// Extract row indecies from 'l', coresponding to node IDs.
//		info = GrB_Matrix_extractTuples_BOOL(mapping, GrB_NULL, GrB_NULL, &n, l);
//		ASSERT(info == GrB_SUCCESS);
//
//		info = GrB_Matrix_extract(reduced, GrB_NULL, GrB_NULL, r, mapping, n,
//								  mapping, n, GrB_NULL);
//		ASSERT(info == GrB_SUCCESS);
//
//		r = reduced;
//	} else {
//		//----------------------------------------------------------------------
//		// Make a copy of the relationship matrix,
//		// truncating it to remove unused rows and casting it to boolean.
//		//----------------------------------------------------------------------
//		info = GrB_Matrix_nrows(&nrows, r);
//		ASSERT(info == GrB_SUCCESS);
//
//		// Build a new boolean matrix with the same dimensions of the original
//		// with all values casted to boolean.
//		GrB_Matrix r_trimmed;
//		info = GrB_Matrix_dup(&r_trimmed, r);
//		ASSERT(info == GrB_SUCCESS);
//
//		// Resize to remove unused rows.
//		n = Graph_UncompactedNodeCount(g);
//		GxB_Matrix_resize(r_trimmed, n, n);
//		r = r_trimmed;
//	}
//
//	// Invoke Pagerank only if 'r' contains entries.
//	info = GrB_Matrix_nvals(&nvals, r);
//	ASSERT(info == GrB_SUCCESS);
//
//	if(nvals > 0) {
//		info = Pagerank(&ranking, r, itermax, tol, &iters);
//		ASSERT(info == GrB_SUCCESS);
//	}
//
//	// Clean up.
//	GrB_free(&r);
//
//	// Update context.
//	pdata->n = n;
//	pdata->mapping = mapping;
//	pdata->ranking = ranking;
//	return PROCEDURE_OK;
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
	ProcedureOutput *outputs = array_new(ProcedureOutput, 2);
	ProcedureOutput output_node = {.name = "node", .type = T_NODE};
	ProcedureOutput output_score = {.name = "score", .type = T_DOUBLE};
	array_append(outputs, output_node);
	array_append(outputs, output_score);

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

