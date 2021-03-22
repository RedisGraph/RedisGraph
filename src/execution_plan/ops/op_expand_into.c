/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_expand_into.h"
#include "shared/print_functions.h"
#include "../../query_ctx.h"

// default number of records to accumulate before traversing
#define BATCH_SIZE 16

/* Forward declarations. */
static OpResult ExpandIntoInit(OpBase *opBase);
static Record ExpandIntoConsume(OpBase *opBase);
static OpResult ExpandIntoReset(OpBase *opBase);
static OpBase *ExpandIntoClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ExpandIntoFree(OpBase *opBase);

// String representation of operation.
static inline int ExpandIntoToString(const OpBase *ctx, char *buf, uint buf_len) {
	return TraversalToString(ctx, buf, buf_len, ((const OpExpandInto *)ctx)->ae);
}

static void _populate_filter_matrix(OpExpandInto *op) {
	for(uint i = 0; i < op->record_count; i++) {
		Record r = op->records[i];
		/* Update filter matrix F, set row i at position srcId
		 * F[i, srcId] = true. */
		Node *n = Record_GetNode(r, op->srcNodeIdx);
		NodeID srcId = ENTITY_GET_ID(n);
		GrB_Matrix_setElement_BOOL(op->F, true, i, srcId);
	}
}

/* Evaluate algebraic expression:
 * appends filter matrix as the left most operand
 * perform multiplications.
 * removed filter matrix from original expression
 * clears filter matrix. */
static void _traverse(OpExpandInto *op) {
	// If op->F is null, this is the first time we are traversing.
	if(op->F == GrB_NULL) {
		// Create both filter and result matrices.
		size_t required_dim = Graph_RequiredMatrixDim(op->graph);
		GrB_Matrix_new(&op->M, GrB_BOOL, op->record_cap, required_dim);
		GrB_Matrix_new(&op->F, GrB_BOOL, op->record_cap, required_dim);

		// Prepend the filter matrix to algebraic expression as the leftmost operand.
		AlgebraicExpression_MultiplyToTheLeft(&op->ae, op->F);

		// Optimize the expression tree.
		AlgebraicExpression_Optimize(&op->ae);
	}

	// Populate filter matrix.
	_populate_filter_matrix(op);

	// Evaluate expression.
	AlgebraicExpression_Eval(op->ae, op->M, NULL);

	// Clear filter matrix.
	GrB_Matrix_clear(op->F);
}

OpBase *NewExpandIntoOp(const ExecutionPlan *plan, Graph *g, AlgebraicExpression *ae) {
	OpExpandInto *op = rm_malloc(sizeof(OpExpandInto));
	op->graph = g;
	op->ae = ae;
	op->r = NULL;
	op->F = GrB_NULL;
	op->M = GrB_NULL;
	op->records = NULL;
	op->record_cap = BATCH_SIZE;
	op->record_count = 0;
	op->edge_ctx = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_EXPAND_INTO, "Expand Into", ExpandIntoInit, ExpandIntoConsume,
				ExpandIntoReset, ExpandIntoToString, ExpandIntoClone, ExpandIntoFree, false, plan);

	// Make sure that all entities are represented in Record
	bool aware;
	UNUSED(aware);
	aware = OpBase_Aware((OpBase *)op, AlgebraicExpression_Source(ae), &op->srcNodeIdx);
	ASSERT(aware);
	aware = OpBase_Aware((OpBase *)op, AlgebraicExpression_Destination(ae), &op->destNodeIdx);
	ASSERT(aware);

	const char *edge = AlgebraicExpression_Edge(ae);
	if(edge) {
		/* This operation will populate an edge in the Record.
		 * Prepare all necessary information for collecting matching edges. */
		uint edge_idx = OpBase_Modifies((OpBase *)op, edge);
		QGEdge *e = QueryGraph_GetEdgeByAlias(plan->query_graph, edge);
		op->edge_ctx = Traverse_NewEdgeCtx(ae, e, edge_idx);
	}

	return (OpBase *)op;
}

static OpResult ExpandIntoInit(OpBase *opBase) {
	OpExpandInto *op = (OpExpandInto *)opBase;
	// Create 'records' with this Init function as 'record_cap'
	// might be set during optimization time (applyLimit)
	// If cap greater than BATCH_SIZE is specified,
	// use BATCH_SIZE as the value.
	if(op->record_cap > BATCH_SIZE) op->record_cap = BATCH_SIZE;
	op->records = rm_calloc(op->record_cap, sizeof(Record));
	return OP_OK;
}

/* Emits a record when possible,
 * Returns NULL when we've got no more records. */
static Record _handoff(OpExpandInto *op) {
	/* If we're required to update an edge and have one queued, we can return early.
	 * Otherwise, try to get a new pair of source and destination nodes. */
	if(op->edge_ctx && Traverse_SetEdge(op->edge_ctx, op->r)) return OpBase_CloneRecord(op->r);

	/* Find a record where both record's source and destination
	 * nodes are connected. */
	while(op->record_count) {
		op->record_count--;
		// Current record resides at row record_count.
		uint rowIdx = op->record_count;
		op->r = op->records[op->record_count];
		Node *destNode = Record_GetNode(op->r, op->destNodeIdx);
		NodeID destId = ENTITY_GET_ID(destNode);
		bool x;
		GrB_Info res = GrB_Matrix_extractElement_BOOL(&x, op->M, rowIdx, destId);
		// Src is not connected to dest, free the current record and continue.
		if(res != GrB_SUCCESS) {
			OpBase_DeleteRecord(op->r);
			continue;
		}

		// If we're here, src is connected to dest. Update the edge if necessary.
		if(op->edge_ctx) {
			Node *srcNode = Record_GetNode(op->r, op->srcNodeIdx);
			// Collect all appropriate edges connecting the current pair of endpoints.
			Traverse_CollectEdges(op->edge_ctx, ENTITY_GET_ID(srcNode), destId);
			// Add an edge to the Record.
			Traverse_SetEdge(op->edge_ctx, op->r);
			return OpBase_CloneRecord(op->r);
		}

		// Mark as NULL to avoid double free.
		op->records[op->record_count] = NULL;
		return op->r;
	}

	// Didn't manage to emit record.
	return NULL;
}

/* ExpandIntoConsume next operation
 * returns OP_DEPLETED when no additional updates are available */
static Record ExpandIntoConsume(OpBase *opBase) {
	Record r;
	OpExpandInto *op = (OpExpandInto *)opBase;
	OpBase *child = op->op.children[0];

	// As long as we don't have a record to emit.
	while((r = _handoff(op)) == NULL) {
		/* If we're here, we didn't manage to emit a record.
		 * Clean up and try to get new data points. */
		op->r = NULL;
		for(uint i = 0; i < op->record_count; i++) OpBase_DeleteRecord(op->records[i]);

		// Ask child operations for data.
		for(op->record_count = 0; op->record_count < op->record_cap; op->record_count++) {
			Record childRecord = OpBase_Consume(child);
			// Did not manage to get new data, break.
			if(!childRecord) break;
			if(!Record_GetNode(childRecord, op->srcNodeIdx) ||
			   !Record_GetNode(childRecord, op->destNodeIdx)) {
				/* The child Record may not contain the source node in scenarios like
				 * a failed OPTIONAL MATCH. In this case, delete the Record and try again. */
				OpBase_DeleteRecord(childRecord);
				op->record_count--;
				continue;
			}

			// Store received record.
			Record_PersistScalars(childRecord);
			op->records[op->record_count] = childRecord;
		}

		// Depleted.
		if(op->record_count == 0) return NULL;
		_traverse(op);
	}

	return r;
}

static OpResult ExpandIntoReset(OpBase *ctx) {
	OpExpandInto *op = (OpExpandInto *)ctx;
	op->r = NULL;
	for(uint i = 0; i < op->record_count; i++) {
		if(op->records[i]) {
			OpBase_DeleteRecord(op->records[i]);
			op->records[i] = NULL;
		}
	}
	op->record_count = 0;

	if(op->edge_ctx) Traverse_ResetEdgeCtx(op->edge_ctx);
	if(op->F != GrB_NULL) GrB_Matrix_clear(op->F);
	return OP_OK;
}

static inline OpBase *ExpandIntoClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_EXPAND_INTO);
	OpExpandInto *op = (OpExpandInto *)opBase;
	return NewExpandIntoOp(plan, QueryCtx_GetGraph(), AlgebraicExpression_Clone(op->ae));
}

/* Frees ExpandInto */
static void ExpandIntoFree(OpBase *ctx) {
	OpExpandInto *op = (OpExpandInto *)ctx;
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

	if(op->edge_ctx) {
		Traverse_FreeEdgeCtx(op->edge_ctx);
		op->edge_ctx = NULL;
	}

	if(op->records) {
		for(uint i = 0; i < op->record_count; i++) {
			if(op->records[i]) OpBase_DeleteRecord(op->records[i]);
		}
		rm_free(op->records);
		op->records = NULL;
	}
}

