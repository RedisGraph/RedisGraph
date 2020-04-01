/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_conditional_traverse.h"
#include "shared/print_functions.h"
#include "../../util/arr.h"
#include "../../GraphBLASExt/GxB_Delete.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult CondTraverseInit(OpBase *opBase);
static Record CondTraverseConsume(OpBase *opBase);
static OpResult CondTraverseReset(OpBase *opBase);
static OpBase *CondTraverseClone(const ExecutionPlan *plan, const OpBase *opBase);
static void CondTraverseFree(OpBase *opBase);

static inline int CondTraverseToString(const OpBase *ctx, char *buf, uint buf_len) {
	return TraversalToString(ctx, buf, buf_len, ((const CondTraverse *)ctx)->ae);
}

static void _populate_filter_matrix(CondTraverse *op) {
	for(uint i = 0; i < op->recordCount; i++) {
		Record r = op->records[i];
		/* Update filter matrix F, set row i at position srcId
		 * F[i, srcId] = true. */
		Node *n = Record_GetNode(r, op->srcNodeIdx);
		NodeID srcId = ENTITY_GET_ID(n);
		GrB_Matrix_setElement_BOOL(op->F, true, i, srcId);
	}
}

/* Evaluate algebraic expression:
 * prepends filter matrix as the left most operand
 * perform multiplications
 * set iterator over result matrix
 * removed filter matrix from original expression
 * clears filter matrix. */
void _traverse(CondTraverse *op) {
	// Create both filter and result matrices.
	if(op->F == GrB_NULL) {
		size_t required_dim = Graph_RequiredMatrixDim(op->graph);
		GrB_Matrix_new(&op->M, GrB_BOOL, op->recordsCap, required_dim);
		GrB_Matrix_new(&op->F, GrB_BOOL, op->recordsCap, required_dim);
	}

	// Populate filter matrix.
	_populate_filter_matrix(op);
	// Clone expression, as we're about to modify the structure with Optimize.
	AlgebraicExpression *clone = AlgebraicExpression_Clone(op->ae);
	// Prepend matrix to algebraic expression, as the left most operand.
	AlgebraicExpression_MultiplyToTheLeft(&clone, op->F);
	// TODO: consider performing optimization as part of evaluation.
	AlgebraicExpression_Optimize(&clone);
	// Evaluate expression.
	AlgebraicExpression_Eval(clone, op->M);
	// Free clone.
	AlgebraicExpression_Free(clone);

	if(op->iter == NULL) GxB_MatrixTupleIter_new(&op->iter, op->M);
	else GxB_MatrixTupleIter_reuse(op->iter, op->M);

	// Clear filter matrix.
	GrB_Matrix_clear(op->F);
}

OpBase *NewCondTraverseOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae) {
	CondTraverse *op = rm_calloc(1, sizeof(CondTraverse));
	op->graph = g;
	op->ae = ae;
	op->r = NULL;
	op->iter = NULL;
	op->F = GrB_NULL;
	op->M = GrB_NULL;
	op->recordCount = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CONDITIONAL_TRAVERSE, "Conditional Traverse", CondTraverseInit,
				CondTraverseConsume, CondTraverseReset, CondTraverseToString, CondTraverseClone, CondTraverseFree,
				false, plan);

	assert(OpBase_Aware((OpBase *)op, AlgebraicExpression_Source(ae), &op->srcNodeIdx));
	op->destNodeIdx = OpBase_Modifies((OpBase *)op, AlgebraicExpression_Destination(ae));

	const char *edge = AlgebraicExpression_Edge(ae);
	if(edge) {
		op->setEdge = true;
		/* This operation will populate an edge in the Record.
		 * Prepare all necessary information for collecting matching edges. */
		uint edge_idx = OpBase_Modifies((OpBase *)op, edge);
		QGEdge *e = QueryGraph_GetEdgeByAlias(plan->query_graph, edge);
		Traverse_NewEdgeData(&op->edge_data, ae, e, edge_idx);
	}

	return (OpBase *)op;
}

static OpResult CondTraverseInit(OpBase *opBase) {
	CondTraverse *op = (CondTraverse *)opBase;
	AST *ast = ExecutionPlan_GetAST(opBase->plan);
	op->recordsCap = TraverseRecordCap(ast);
	op->records = rm_calloc(op->recordsCap, sizeof(Record));
	return OP_OK;
}

/* Each call to CondTraverseConsume emits a Record containing the
 * traversal's endpoints and, if required, an edge.
 * Returns NULL once all traversals have been performed. */
static Record CondTraverseConsume(OpBase *opBase) {
	CondTraverse *op = (CondTraverse *)opBase;
	OpBase *child = op->op.children[0];

	/* If we're required to update an edge and have one queued, we can return early.
	 * Otherwise, try to get a new pair of source and destination nodes. */
	if(op->setEdge && Traverse_SetEdge(&op->edge_data, op->r)) return OpBase_CloneRecord(op->r);

	bool depleted = true;
	NodeID src_id = INVALID_ENTITY_ID;
	NodeID dest_id = INVALID_ENTITY_ID;

	while(true) {
		if(op->iter) GxB_MatrixTupleIter_next(op->iter, &src_id, &dest_id, &depleted);

		// Managed to get a tuple, break.
		if(!depleted) break;

		/* Run out of tuples, try to get new data.
		 * Free old records. */
		op->r = NULL;
		for(uint i = 0; i < op->recordCount; i++) OpBase_DeleteRecord(op->records[i]);

		// Ask child operations for data.
		for(op->recordCount = 0; op->recordCount < op->recordsCap; op->recordCount++) {
			Record childRecord = OpBase_Consume(child);
			// If the Record is NULL, the child has been depleted.
			if(!childRecord) break;
			if(!Record_GetNode(childRecord, op->srcNodeIdx)) {
				/* The child Record may not contain the source node in scenarios like
				 * a failed OPTIONAL MATCH. In this case, delete the Record and try again. */
				OpBase_DeleteRecord(childRecord);
				op->recordsLen--;
				continue;
			}

			// Store received record.
			Record_PersistScalars(childRecord);
			op->records[op->recordCount] = childRecord;
		}

		// No data.
		if(op->recordCount == 0) return NULL;

		_traverse(op);
	}

	/* Get node from current column. */
	op->r = op->records[src_id];
	Node destNode = {0};
	Graph_GetNode(op->graph, dest_id, &destNode);
	Record_AddNode(op->r, op->destNodeIdx, destNode);

	if(op->setEdge) {
		Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);
		// Collect all appropriate edges connecting the current pair of endpoints.
		Traverse_CollectEdges(&op->edge_data, ENTITY_GET_ID(srcNode), ENTITY_GET_ID(destNode));
		// We're guaranteed to have at least one edge.
		Traverse_SetEdge(&op->edge_data, op->r);
	}

	return OpBase_CloneRecord(op->r);
}

static OpResult CondTraverseReset(OpBase *ctx) {
	CondTraverse *op = (CondTraverse *)ctx;

	// Do not explicitly free op->r, as the same pointer is also held
	// in the op->records array and as such will be freed there.
	op->r = NULL;
	for(uint i = 0; i < op->recordCount; i++) OpBase_DeleteRecord(op->records[i]);
	op->recordCount = 0;

	if(op->setEdge) array_clear(op->edge_data.edges);
	if(op->iter) {
		GxB_MatrixTupleIter_free(op->iter);
		op->iter = NULL;
	}
	if(op->F != GrB_NULL) GrB_Matrix_clear(op->F);
	return OP_OK;
}

static inline OpBase *CondTraverseClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_CONDITIONAL_TRAVERSE);
	CondTraverse *op = (CondTraverse *)opBase;
	return NewCondTraverseOp(plan, QueryCtx_GetGraph(), AlgebraicExpression_Clone(op->ae));
}

/* Frees CondTraverse */
static void CondTraverseFree(OpBase *ctx) {
	CondTraverse *op = (CondTraverse *)ctx;
	if(op->iter) {
		GxB_MatrixTupleIter_free(op->iter);
		op->iter = NULL;
	}

	if(op->F != GrB_NULL) {
		GrB_Matrix_free(&op->F);
		op->F = GrB_NULL;
	}

	if(op->M != GrB_NULL) {
		GrB_Matrix_free(&op->M);
		op->M = GrB_NULL;
	}

	if(op->ae) {
		AlgebraicExpression_Free(op->ae);
		op->ae = NULL;
	}

	if(op->setEdge) Traverse_FreeEdgeData(&op->edge_data);

	if(op->records) {
		for(uint i = 0; i < op->recordCount; i++) OpBase_DeleteRecord(op->records[i]);
		rm_free(op->records);
		op->records = NULL;
	}
}

