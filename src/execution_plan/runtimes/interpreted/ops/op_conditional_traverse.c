/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "RG.h"
#include "../../../ops/shared/print_functions.h"
#include "../../../../query_ctx.h"

// default number of records to accumulate before traversing
#define BATCH_SIZE 16

/* Forward declarations. */
static RT_OpResult CondTraverseInit(RT_OpBase *opBase);
static Record CondTraverseConsume(RT_OpBase *opBase);
static RT_OpResult CondTraverseReset(RT_OpBase *opBase);
static void CondTraverseFree(RT_OpBase *opBase);

static void _populate_filter_matrix(RT_OpCondTraverse *op) {
	GrB_Matrix FM = RG_MATRIX_M(op->F);

	for(uint i = 0; i < op->record_count; i++) {
		Record r = op->records[i];
		/* Update filter matrix F, set row i at position srcId
		 * F[i, srcId] = true. */
		Node *n = Record_GetNode(r, op->srcNodeIdx);
		NodeID srcId = ENTITY_GET_ID(n);	
		GrB_Matrix_setElement_BOOL(FM, true, i, srcId);
	}

	GrB_Matrix_wait(&FM);
}

/* Evaluate algebraic expression:
 * prepends filter matrix as the left most operand
 * perform multiplications
 * set iterator over result matrix
 * removed filter matrix from original expression
 * clears filter matrix. */
void _traverse(RT_OpCondTraverse *op) {
	// If op->F is null, this is the first time we are traversing.
	if(op->F == NULL) {
		/* Create both filter and result matrices.
		 * make sure M's format is SPARSE, required by the matrix iterator */
		size_t required_dim = Graph_RequiredMatrixDim(op->graph);
		RG_Matrix_new(&op->M, GrB_BOOL, op->record_cap, required_dim);
		RG_Matrix_new(&op->F, GrB_BOOL, op->record_cap, required_dim);

		// Prepend the filter matrix to algebraic expression as the leftmost operand.
		AlgebraicExpression_MultiplyToTheLeft(&op->ae, op->F);

		// Optimize the expression tree.
		AlgebraicExpression_Optimize(&op->ae);
	}

	// Populate filter matrix.
	_populate_filter_matrix(op);

	// Evaluate expression.
	AlgebraicExpression_Eval(op->ae, op->M);

	if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, RG_MATRIX_M(op->M));
	else GxB_MatrixTupleIter_reuse(op->iter, RG_MATRIX_M(op->M));

	// Clear filter matrix.
	RG_Matrix_clear(op->F);
}

RT_OpBase *RT_NewCondTraverseOp(const RT_ExecutionPlan *plan, const OpCondTraverse *op_desc) {
	RT_OpCondTraverse *op = rm_malloc(sizeof(RT_OpCondTraverse));
	op->op_desc = op_desc;
	op->ae = AlgebraicExpression_Clone(op_desc->ae);
	op->r = NULL;
	op->iter = NULL;
	op->F = NULL;
	op->M = NULL;
	op->records = NULL;
	op->record_count = 0;
	op->edge_ctx = NULL;
	op->record_cap = BATCH_SIZE;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, CondTraverseInit,
				CondTraverseConsume, CondTraverseReset, CondTraverseFree, plan);

	bool aware = RT_OpBase_Aware((RT_OpBase *)op, AlgebraicExpression_Source(op->ae), &op->srcNodeIdx);
	UNUSED(aware);
	ASSERT(aware == true);

	const char *dest = AlgebraicExpression_Destination(op->ae);
	aware = RT_OpBase_Aware((RT_OpBase *)op, dest, &op->destNodeIdx);
	ASSERT(aware);

	const char *edge = AlgebraicExpression_Edge(op->ae);
	if(edge) {
		/* This operation will populate an edge in the Record.
		 * Prepare all necessary information for collecting matching edges. */
		uint edge_idx;
		aware = RT_OpBase_Aware((RT_OpBase *)op, edge, &edge_idx);
		QGEdge *e = QueryGraph_GetEdgeByAlias(plan->plan_desc->query_graph, edge);
		op->edge_ctx = Traverse_NewEdgeCtx(op->ae, e, edge_idx);
	}

	return (RT_OpBase *)op;
}

static RT_OpResult CondTraverseInit(RT_OpBase *opBase) {
	RT_OpCondTraverse *op = (RT_OpCondTraverse *)opBase;
	op->graph = QueryCtx_GetGraph();
	// Create 'records' with this Init function as 'record_cap'
	// might be set during optimization time (applyLimit)
	// If cap greater than BATCH_SIZE is specified,
	// use BATCH_SIZE as the value.
	if(op->record_cap > BATCH_SIZE) op->record_cap = BATCH_SIZE;
	op->records = rm_calloc(op->record_cap, sizeof(Record));
	return OP_OK;
}

/* Each call to CondTraverseConsume emits a Record containing the
 * traversal's endpoints and, if required, an edge.
 * Returns NULL once all traversals have been performed. */
static Record CondTraverseConsume(RT_OpBase *opBase) {
	RT_OpCondTraverse *op = (RT_OpCondTraverse *)opBase;
	RT_OpBase *child = op->op.children[0];

	/* If we're required to update an edge and have one queued, we can return early.
	 * Otherwise, try to get a new pair of source and destination nodes. */
	if(op->edge_ctx && Traverse_SetEdge(op->edge_ctx, op->r)) return RT_OpBase_CloneRecord(op->r);

	bool depleted = true;
	NodeID src_id = INVALID_ENTITY_ID;
	NodeID dest_id = INVALID_ENTITY_ID;

	while(true) {
		if(op->iter) GxB_MatrixTupleIter_next(op->iter, &src_id, &dest_id,
				NULL, &depleted);

		// Managed to get a tuple, break.
		if(!depleted) break;

		/* Run out of tuples, try to get new data.
		 * Free old records. */
		op->r = NULL;
		for(uint i = 0; i < op->record_count; i++) RT_OpBase_DeleteRecord(op->records[i]);

		// Ask child operations for data.
		for(op->record_count = 0; op->record_count < op->record_cap; op->record_count++) {
			Record childRecord = RT_OpBase_Consume(child);
			// If the Record is NULL, the child has been depleted.
			if(!childRecord) break;
			if(!Record_GetNode(childRecord, op->srcNodeIdx)) {
				/* The child Record may not contain the source node in scenarios like
				 * a failed OPTIONAL MATCH. In this case, delete the Record and try again. */
				RT_OpBase_DeleteRecord(childRecord);
				op->record_count--;
				continue;
			}

			// Store received record.
			Record_PersistScalars(childRecord);
			op->records[op->record_count] = childRecord;
		}

		// No data.
		if(op->record_count == 0) return NULL;

		_traverse(op);
	}

	/* Get node from current column. */
	op->r = op->records[src_id];
	/* Populate the destination node and add it to the Record.
	 * Note that if the node's label is unknown, this will correctly
	 * create an unlabeled node. */
	Node destNode = GE_NEW_LABELED_NODE(op->op_desc->dest_label, op->op_desc->dest_label_id);
	Graph_GetNode(op->graph, dest_id, &destNode);
	Record_AddNode(op->r, op->destNodeIdx, destNode);

	if(op->edge_ctx) {
		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);
		// Collect all appropriate edges connecting the current pair of endpoints.
		Traverse_CollectEdges(op->edge_ctx, ENTITY_GET_ID(srcNode), ENTITY_GET_ID(&destNode));
		// We're guaranteed to have at least one edge.
		Traverse_SetEdge(op->edge_ctx, op->r);
	}

	return RT_OpBase_CloneRecord(op->r);
}

static RT_OpResult CondTraverseReset(RT_OpBase *ctx) {
	RT_OpCondTraverse *op = (RT_OpCondTraverse *)ctx;

	// Do not explicitly free op->r, as the same pointer is also held
	// in the op->records array and as such will be freed there.
	op->r = NULL;
	for(uint i = 0; i < op->record_count; i++) RT_OpBase_DeleteRecord(op->records[i]);
	op->record_count = 0;

	if(op->edge_ctx) Traverse_ResetEdgeCtx(op->edge_ctx);

	if(op->iter) {
		GxB_MatrixTupleIter_free(&op->iter);
	}
	if(op->F != NULL) RG_Matrix_clear(op->F);
	return OP_OK;
}

/* Frees CondTraverse */
static void CondTraverseFree(RT_OpBase *ctx) {
	RT_OpCondTraverse *op = (RT_OpCondTraverse *)ctx;
	if(op->iter) {
		GxB_MatrixTupleIter_free(&op->iter);
	}

	if(op->F != NULL) {
		RG_Matrix_free(&op->F);
		op->F = NULL;
	}

	if(op->M != NULL) {
		RG_Matrix_free(&op->M);
		op->M = NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->edge_ctx) {
		Traverse_FreeEdgeCtx(op->edge_ctx);
		op->edge_ctx = NULL;
	}

	if(op->records) {
		for(uint i = 0; i < op->record_count; i++) RT_OpBase_DeleteRecord(op->records[i]);
		rm_free(op->records);
		op->records = NULL;
	}
}
