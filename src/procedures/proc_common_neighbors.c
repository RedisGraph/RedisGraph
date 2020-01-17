/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "proc_common_neighbors.h"

#include "../util/arr.h"
#include "../query_ctx.h"
#include "../graph/graphcontext.h"
#define BATCH_SIZE 16384

typedef struct {
	GrB_Index nrows;            // Rows in A.
	GrB_Index batch_size;       // Number of rows to process.
	GrB_Matrix A;               // Relationship matrix.
	GrB_Matrix At;              // Transposed relationship matrix.
	GrB_Matrix SIM;             // Similarity matrix, SIM[i,j] = #routes between i and j.
	GrB_Vector DEGREE;          // Out degree, DEGREE[i] = i's out degree.
	GxB_Scalar thunk;           // Row offset.
	GrB_Descriptor desc;        // Descriptor.
	GxB_SelectOp select_op;     // Selects upper triangle.
	GxB_MatrixTupleIter *iter;  // Matrix iterator.
	int iteration;              // Iteration.
	Node node1;                 // Node 1.
	Node node2;                 // Node 2.
	Graph *graph;               // Graph object.
	SIValue *output;            // Array with 6 entries ["node1", node, "node2", node, "score", score].
} SimilarityContext;

// Create a new similarity context.
static SimilarityContext *SimilarityContext_New() {
	SimilarityContext *ctx = rm_malloc(sizeof(SimilarityContext));
	ctx->nrows = 0;
	ctx->batch_size = 0;
	ctx->iter = NULL;
	ctx->iteration = -1;
	ctx->A = GrB_NULL;
	ctx->At = GrB_NULL;
	ctx->SIM = GrB_NULL;
	ctx->desc = GrB_NULL;
	ctx->thunk = GrB_NULL;
	ctx->DEGREE = GrB_NULL;
	ctx->select_op = GrB_NULL;
	ctx->graph = QueryCtx_GetGraph();

	// Setup reusable output.
	ctx->output = array_new(SIValue, 6);
	ctx->output = array_append(ctx->output, SI_ConstStringVal("node1"));
	ctx->output = array_append(ctx->output, SI_Node(NULL)); // Place holder.
	ctx->output = array_append(ctx->output, SI_ConstStringVal("node2"));
	ctx->output = array_append(ctx->output, SI_Node(NULL)); // Place holder.
	ctx->output = array_append(ctx->output, SI_ConstStringVal("score"));
	ctx->output = array_append(ctx->output, SI_DoubleVal(0.0)); // Place holder.

	return ctx;
}

// Free similarity context.
static void SimilarityContext_Free(SimilarityContext *ctx) {
	if(ctx) {
		if(ctx->output) array_free(ctx->output);
		if(ctx->At != GrB_NULL) GrB_free(&ctx->At);
		if(ctx->SIM != GrB_NULL) GrB_free(&ctx->SIM);
		if(ctx->desc != GrB_NULL) GrB_free(&ctx->desc);
		if(ctx->thunk != GrB_NULL) GrB_free(&ctx->thunk);
		if(ctx->DEGREE != GrB_NULL) GrB_free(&ctx->DEGREE);
		if(ctx->select_op != GrB_NULL) GrB_free(&ctx->select_op);
		if(ctx->iter) GxB_MatrixTupleIter_free(ctx->iter);
		rm_free(ctx);
	}
}

/* Selects the upper triangle portion of a matrix.
 * taking into account row offset. */
bool _select_triu(GrB_Index i, GrB_Index j, GrB_Index nrows, GrB_Index ncols, const void *x,
				  const void *thunk) {

	int64_t offset = *((int64_t *)thunk);
	// Upper triangular.
	return j > (offset + i);
}

// Extract row min through max from `A`, store result is `C`.
static void _extract_rows(GrB_Matrix C, GrB_Matrix A, GrB_Index min, GrB_Index max) {
	assert(C && A && (min <= max));

	GrB_Info info;
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix_nrows(&nrows, A);
	GrB_Matrix_ncols(&ncols, A);

	max = MIN(max, nrows);

	// Build array of row indices to extract.
	GrB_Index count = max - min;
	GrB_Index rows[count];
	for(GrB_Index i = 0; i < count; i++) rows[i] = min + i;

	info = GrB_Matrix_nrows(&nrows, C);
	assert(info == GrB_SUCCESS);

	// Make sure C dimensions matches.
	if(nrows != count) {
		info = GxB_Matrix_resize(C, count, ncols);
		assert(info == GrB_SUCCESS);
	}

	// Extract row min through max.
	info = GrB_Matrix_extract(C, GrB_NULL, GrB_NULL, A, rows, count, GrB_ALL, ncols, GrB_NULL);
	assert(info == GrB_SUCCESS);
}

static void _reiterate(SimilarityContext *ctx) {
	GrB_Info info;
	GrB_Index nvals;

	GrB_Index min = ctx->iteration * ctx->batch_size;
	GrB_Index max = min + ctx->batch_size;
	_extract_rows(ctx->SIM, ctx->A, min, max);

	// Compute (f * A) * At, [A*At][i,j] = Number of paths between i and j.
	info = GrB_mxm(ctx->SIM, GrB_NULL, GrB_NULL, GxB_PLUS_TIMES_INT64, ctx->SIM, ctx->At, GrB_NULL);
	assert(info == GrB_SUCCESS);

	info = GxB_Scalar_setElement_INT64(ctx->thunk, min);
	assert(info == GrB_SUCCESS);

	info = GxB_Matrix_select(ctx->SIM, GrB_NULL, GrB_NULL, ctx->select_op, ctx->SIM, ctx->thunk,
							 ctx->desc);
	assert(info == GrB_SUCCESS);

	info = GrB_Matrix_nvals(&nvals, ctx->SIM);
	assert(info == GrB_SUCCESS);

	// Force flush.
	GrB_Matrix_nvals(&nvals, ctx->SIM);

	// Reset iterator.
	if(!ctx->iter) GxB_MatrixTupleIter_new(&ctx->iter, ctx->SIM);
	else GxB_MatrixTupleIter_reuse(ctx->iter, ctx->SIM);
}

ProcedureResult Proc_CommonNeighborsInvoke(ProcedureCtx *ctx, const SIValue *args) {
	if(array_len((SIValue *)args) != 1) return PROCEDURE_ERR;
	if(!(SI_TYPE(args[0]) & T_STRING)) return PROCEDURE_ERR;

	const char *relation = args[0].stringval;

	GrB_Info info;
	GrB_Index nvals;
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Index batch_size;
	GrB_Matrix A = GrB_NULL;
	GrB_Matrix At = GrB_NULL;
	GrB_Matrix SIM = GrB_NULL;
	GxB_Scalar thunk = GrB_NULL;
	GrB_Vector DEGREE = GrB_NULL;
	GrB_Descriptor desc = GrB_NULL;
	GxB_SelectOp select_op = GrB_NULL;

	Graph *g = QueryCtx_GetGraph();
	GraphContext *gc = QueryCtx_GetGraphCtx();

	Schema *s = GraphContext_GetSchema(gc, relation, SCHEMA_EDGE);
	if(!s) return PROCEDURE_OK;
	A = Graph_GetRelationMatrix(g, s->id);

	info = GrB_Matrix_nrows(&nrows, A);
	assert(info == GrB_SUCCESS);

	info = GrB_Matrix_ncols(&ncols, A);
	assert(info == GrB_SUCCESS);

	batch_size = MIN(nrows, BATCH_SIZE);

	// Create similarity matrix.
	info = GrB_Matrix_new(&SIM, GrB_INT64, batch_size, ncols);
	assert(info == GrB_SUCCESS);

	// Create transposed relationship matrix.
	info = GrB_Matrix_new(&At, GrB_BOOL, nrows, ncols);
	assert(info == GrB_SUCCESS);

	// Transpose relationship matrix.
	info = GrB_transpose(At, GrB_NULL, GrB_NULL, A, GrB_NULL);
	assert(info == GrB_SUCCESS);

	// Create degree vector, DEGREE[i] = out degree of node i.
	info = GrB_Vector_new(&DEGREE, GrB_INT64, ncols);
	assert(info == GrB_SUCCESS);

	// Reduce A rows, using plus to sum each row.
	info = GrB_Matrix_reduce_Monoid(DEGREE, GrB_NULL, GrB_NULL, GxB_PLUS_INT64_MONOID, A, GrB_NULL);
	assert(info == GrB_SUCCESS);

	info = GxB_Scalar_new(&thunk, GrB_INT64);
	assert(info == GrB_SUCCESS);

	info = GxB_SelectOp_new(&select_op, _select_triu, GrB_INT64, GrB_INT64);
	assert(info == GrB_SUCCESS);

	info = GrB_Descriptor_new(&desc);
	assert(info == GrB_SUCCESS);
	GrB_Descriptor_set(desc, GrB_OUTP, GrB_REPLACE);

	// Update procedure context.
	SimilarityContext *sim_ctx = SimilarityContext_New();
	sim_ctx->A = A;
	sim_ctx->At = At;
	sim_ctx->SIM = SIM;
	sim_ctx->desc = desc;
	sim_ctx->nrows = nrows;
	sim_ctx->thunk = thunk;
	sim_ctx->select_op = select_op;
	sim_ctx->batch_size = batch_size;
	sim_ctx->DEGREE = DEGREE;

	ctx->privateData = sim_ctx;
	return PROCEDURE_OK;
}

SIValue *Proc_CommonNeighborsStep(ProcedureCtx *ctx) {
	assert(ctx->privateData);
	SimilarityContext *pdata = ctx->privateData;
	GrB_Matrix SIM = pdata->SIM;
	GrB_Vector DEGREE = pdata->DEGREE;

	bool depleted = true;
	GrB_Index row;
	GrB_Index col;
	NodeID src_id = INVALID_ENTITY_ID;
	NodeID dest_id = INVALID_ENTITY_ID;

	GxB_MatrixTupleIter_next(pdata->iter, &row, &col, &depleted);

	uint64_t offset = pdata->iteration * pdata->batch_size;
	while(depleted) {
		pdata->iteration++;
		offset = pdata->iteration * pdata->batch_size;
		if(offset >= pdata->nrows) return NULL;
		// Reiterate.
		_reiterate(pdata);
		GxB_MatrixTupleIter_next(pdata->iter, &row, &col, &depleted);
	}

	src_id = row + offset;
	dest_id = col;
	Graph_GetNode(pdata->graph, src_id, &pdata->node1);
	Graph_GetNode(pdata->graph, dest_id, &pdata->node2);

	/* Compute similarity ratio string
	 * intersection / union - intersection */
	int64_t i;          // Intersection.
	int64_t u;          // Union.
	int64_t src_degree; // Source out degree.
	int64_t dest_degree; // Destination out degree.

	GrB_Matrix_extractElement_INT64(&i, SIM, row, col);
	GrB_Vector_extractElement_INT64(&src_degree, DEGREE, src_id);
	GrB_Vector_extractElement_INT64(&dest_degree, DEGREE, dest_id);
	u = src_degree + dest_degree - i;   // Remove intersection.

	// Set output: node1, node2 and the intersect union ratio.
	pdata->output[1] = SI_Node(&pdata->node1);
	pdata->output[3] = SI_Node(&pdata->node2);
	pdata->output[5] = SI_DoubleVal((double)i / u);

	return pdata->output;
}

ProcedureResult Proc_CommonNeighborsFree(ProcedureCtx *ctx) {
	// Clean up.
	if(ctx->privateData) SimilarityContext_Free((SimilarityContext *)ctx->privateData);
	return PROCEDURE_OK;
}

ProcedureCtx *Proc_CommonNeighborsCtx() {
	void *privateData = NULL;
	ProcedureOutput **outputs = array_new(ProcedureOutput *, 4);
	ProcedureOutput *output_node1 = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_node2 = rm_malloc(sizeof(ProcedureOutput));
	ProcedureOutput *output_score = rm_malloc(sizeof(ProcedureOutput));
	output_node1->name = "node1";
	output_node1->type = T_NODE;
	output_node2->name = "node2";
	output_node2->type = T_NODE;
	output_score->name = "score";
	output_score->type = T_DOUBLE;

	outputs = array_append(outputs, output_node1);
	outputs = array_append(outputs, output_node2);
	outputs = array_append(outputs, output_score);
	ProcedureCtx *ctx = ProcCtxNew("algo.commonNeighbors",
								   1,
								   outputs,
								   Proc_CommonNeighborsStep,
								   Proc_CommonNeighborsInvoke,
								   Proc_CommonNeighborsFree,
								   privateData,
								   true);
	return ctx;
}
