/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_cdlp.h"
#include "../RG.h"
#include "../value.h"
#include "../config.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../datatypes/array.h"
#include "../graph/graphcontext.h"
#include "../algorithms/LAGraph_cdlp.h"

// The CDLP procedure performs community detection by label propagation.
// Its inputs are:
// 1. the maximum number of iterations, -1 for unlimited
// 2. the relationship type to traverse, NULL for type-agnostic
//
// It outputs:
// 1. node - a node in the graph
// 2. community_id - the ID of the community this node belongs to
//
// CALL algo.cdlp(-1, 'MANAGES') YIELD node, community_id

typedef struct {
	Graph *g;                       // Graph scanned.
	Node n;                         // Currently-accessed node
	NodeID cur;                     // ID of current node
	GrB_Index nvals;                // Number of entries in labels matrix
	GrB_Vector labels;              // Vector of all community labels.
	SIValue *output;                // Array with a maximum of 4 entries: ["node", node, "community_id", community ID].
	int node_output_idx;            // Offset of node value in outputs
	int community_output_idx;       // Offset of community label ID in outputs
} CDLPCtx;

static void _process_yield(CDLPCtx *ctx, const char **yield) {
	bool yield_node = false;
	bool yield_community = false;

	if(yield != NULL) {
		for(uint i = 0; i < array_len(yield); i++) {
			if(strcasecmp("node", yield[i]) == 0) {
				yield_node = true;
				continue;
			}
			if(strcasecmp("community_id", yield[i]) == 0) {
				yield_community = true;
				continue;
			}
		}
	} else {
		/* User did not add an explicit YIELD
		 * use the default yields - both nodes and edges. */
		yield_node = true;
		yield_community = true;
	}

	if(yield_node) {
		ctx->output = array_append(ctx->output, SI_ConstStringVal("node"));
		ctx->node_output_idx = array_len(ctx->output);
		ctx->output = array_append(ctx->output, SI_NullVal()); // Place holder.
	}

	if(yield_community) {
		ctx->output = array_append(ctx->output, SI_ConstStringVal("community_id"));
		ctx->community_output_idx = array_len(ctx->output);
		ctx->output = array_append(ctx->output, SI_NullVal()); // Place holder.
	}
}

#include "../algorithms/LAGraph_internal.h" // TODO tmp, move
static ProcedureResult Proc_CDLP_Invoke(ProcedureCtx *ctx,
										const SIValue *args, const char **yield) {
	// Validate inputs
	ASSERT(ctx != NULL);
	ASSERT(args != NULL);

	if(array_len((SIValue *)args) != 3) return PROCEDURE_ERR;
	if(SI_TYPE(args[0]) != T_INT64               || // Maximum number of iterations, -1 for unlimited.
	   !(SI_TYPE(args[1]) & (T_NULL | T_ARRAY))  || // Array of relationship types to consider if not NULL.
	   !(SI_TYPE(args[2]) & (T_NULL | T_ARRAY)))    // Array of labels to consider if not NULL.
		return PROCEDURE_ERR;

	CDLPCtx *cdlp_ctx = ctx->privateData;
	_process_yield(cdlp_ctx, yield);

	LAGraph_Complex_init() ;    // TODO tmp, move
	LAGraph_alloc_global() ;    // TODO tmp, move
	//--------------------------------------------------------------------------
	// Process inputs
	//--------------------------------------------------------------------------
	int64_t max_iters = args[0].longval;
	// if(max_iters == -1) max_iters = INT_MAX;
	if(max_iters == -1) max_iters = 10;
	if(max_iters < 0) return PROCEDURE_ERR;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	GrB_Info res;
	UNUSED(res);
	GrB_Matrix A;
	SIValue reltypes = args[1];
	SIValue labels = args[2];

	GrB_Index dims = Graph_RequiredMatrixDim(gc->g);
	GrB_Matrix_new(&A, GrB_UINT64, dims, dims); // TODO bool?

	if(!SIValue_IsNull(reltypes)) {
		// Add each specified adjacency matrix to A
		uint count = array_len(reltypes.array);
		for(uint i = 0; i < count; i ++) {
			if(SI_TYPE(reltypes.array[i]) != T_STRING) {
				GrB_Matrix_free(&A);
				ErrorCtx_RaiseRuntimeException("Encountered non-string value in relationship types array");
			}
			const char *reltype = reltypes.array[i].stringval;

			Schema *s = GraphContext_GetSchema(gc, reltype, SCHEMA_EDGE);
			if(!s) continue; // Failed to find schema. TODO what should happen here?
			GrB_Matrix R = Graph_GetRelationMatrix(gc->g, s->id);
			res = GrB_eWiseAdd(A, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, A, R, GrB_NULL);
			ASSERT(res == GrB_SUCCESS);
		}
	} else {
		// Add the full adjacency matrix to A
		GrB_Matrix R = Graph_GetAdjacencyMatrix(gc->g);
		res = GrB_eWiseAdd(A, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, A, R, GrB_NULL);
		GxB_print(R, GxB_COMPLETE);
		ASSERT(res == GrB_SUCCESS);
	}

	GrB_Matrix F = GrB_NULL;
	if(!SIValue_IsNull(labels)) {
		GrB_Matrix_new(&F, GrB_BOOL, dims, dims);
		// Multiply A on the left and right by each specified label matrix to filter
		uint count = array_len(labels.array);
		for(uint i = 0; i < count; i ++) {
			if(SI_TYPE(labels.array[i]) != T_STRING) {
				GrB_Matrix_free(&A);
				GrB_Matrix_free(&F);
				ErrorCtx_RaiseRuntimeException("Encountered non-string value in labels array");
			}
			const char *label = labels.array[i].stringval;

			Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
			if(!s) continue; // Failed to find schema. TODO what should happen here?
			GrB_Matrix L = Graph_GetLabelMatrix(gc->g, s->id);

			res = GrB_eWiseAdd(F, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, L, GrB_NULL);
			ASSERT(res == GrB_SUCCESS);

		}
		// Multiply to the left
		res = GrB_mxm(A, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, F, A, GrB_NULL);
		ASSERT(res == GrB_SUCCESS);
		// Multiply to the right
		res = GrB_mxm(A, GrB_NULL, GrB_NULL, GxB_ANY_PAIR_BOOL, A, F, GrB_NULL);
		ASSERT(res == GrB_SUCCESS);
	}

	// Clear the diagonal, as the CDLP algorithm does not support self-directed edges
	res = GxB_Matrix_select(A, GrB_NULL, GrB_NULL, GxB_OFFDIAG, A, GrB_NULL, GrB_NULL);
	ASSERT(res == GrB_SUCCESS);

	GrB_Vector V = GrB_NULL;  // Vector of results
	GxB_print(A, GxB_COMPLETE);
	res = LAGraph_cdlp(&V, A, false, true, max_iters, NULL);
	GxB_print(A, GxB_COMPLETE);
	GxB_print(V, GxB_COMPLETE);
	ASSERT(res == GrB_SUCCESS);
	GrB_Matrix_free(&A);
	if(F != GrB_NULL) {
		// We are filtering by label, remove all nodes with inappropriate labels from
		// the vector with vector-matrix multiplication
		res = GrB_vxm(V, GrB_NULL, GrB_NULL, GrB_MIN_MAX_SEMIRING_UINT64, V, F, GrB_NULL);
		ASSERT(res == GrB_SUCCESS);
		GrB_Matrix_free(&F);
	}
	cdlp_ctx->labels = V;
	GrB_Index nvals;
	GrB_Vector_nvals(&nvals, V);
	cdlp_ctx->nvals = nvals;
	return PROCEDURE_OK;
}

static SIValue *Proc_CDLP_Step(ProcedureCtx *ctx) {
	ASSERT(ctx->privateData);

	CDLPCtx *cdlp_ctx = (CDLPCtx *)ctx->privateData;

	if(cdlp_ctx->cur >= cdlp_ctx->nvals) return NULL; // depleted

	GrB_Index community_id;
	GrB_Info res;
	UNUSED(res);

	res = GrB_Vector_extractElement_UINT64(&community_id, cdlp_ctx->labels, cdlp_ctx->cur);
	ASSERT(res == GrB_SUCCESS);

	NodeID node_id = cdlp_ctx->cur;

	// Increment the current node ID
	cdlp_ctx->cur++;

	// Populate output.
	if(cdlp_ctx->node_output_idx >= 0) {
		// Emit the current node
		cdlp_ctx->n = GE_NEW_NODE();
		Graph_GetNode(cdlp_ctx->g, node_id, &cdlp_ctx->n);
		cdlp_ctx->output[cdlp_ctx->node_output_idx] = SI_Node(&cdlp_ctx->n);
	}

	if(cdlp_ctx->community_output_idx >= 0)
		cdlp_ctx->output[cdlp_ctx->community_output_idx] = SI_LongVal(community_id);

	return cdlp_ctx->output;
}

static ProcedureResult Proc_CDLP_Free(ProcedureCtx *ctx) {
	ASSERT(ctx != NULL);
	// Free private data.
	CDLPCtx *pdata = ctx->privateData;
	if(pdata->output != NULL) array_free(pdata->output);
	if(pdata->labels != NULL) GrB_Vector_free(&pdata->labels);
	rm_free(ctx->privateData);

	return PROCEDURE_OK;
}

static CDLPCtx *_Build_Private_Data() {
	// Set up the CDLP context.
	CDLPCtx *pdata = rm_calloc(1, sizeof(CDLPCtx));
	pdata->cur = 0;
	pdata->labels = GrB_NULL;
	pdata->node_output_idx = -1;
	pdata->community_output_idx = -1;
	pdata->g = QueryCtx_GetGraph();
	pdata->output = array_new(SIValue, 4);
	return pdata;
}

ProcedureCtx *Proc_CDLP_Ctx() {
	// Construct procedure private data.
	void *privdata = _Build_Private_Data();

	// Declare possible outputs.
	ProcedureOutput *outputs = array_new(ProcedureOutput, 2);
	ProcedureOutput out_nodes = {.name = "node", .type = T_NODE};
	ProcedureOutput out_community = {.name = "community_id", .type = T_INT64};
	outputs = array_append(outputs, out_nodes);
	outputs = array_append(outputs, out_community);

	ProcedureCtx *ctx = ProcCtxNew("algo.CDLP",
								   3,
								   outputs,
								   Proc_CDLP_Step,
								   Proc_CDLP_Invoke,
								   Proc_CDLP_Free,
								   privdata,
								   true);
	return ctx;
}

