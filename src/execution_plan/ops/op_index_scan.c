/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_index_scan.h"
#include "shared/print_functions.h"

/* Forward declarations. */
static OpResult IndexScanInit(OpBase *opBase);
static Record IndexScanConsume(OpBase *opBase);
static Record IndexScanConsumeFromChild(OpBase *opBase);
static OpResult IndexScanReset(OpBase *opBase);
static void IndexScanFree(OpBase *opBase);

static int IndexScanToString(const OpBase *ctx, char *buf, uint buf_len) {
	return ScanToString(ctx, buf, buf_len, ((const IndexScan *)ctx)->n);
}

OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, const QGNode *n, RSIndex *idx,
					   RSResultsIterator *iter) {
	IndexScan *op = malloc(sizeof(IndexScan));
	op->g = g;
	op->n = n;
	op->idx = idx;
	op->iter = iter;
	op->child_record = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_INDEX_SCAN, "Index Scan", IndexScanInit, IndexScanConsume,
				IndexScanReset, IndexScanToString, IndexScanFree, plan, false);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);
	return (OpBase *)op;
}

static OpResult IndexScanInit(OpBase *opBase) {
	if(opBase->childCount > 0) opBase->consume = IndexScanConsumeFromChild;
	return OP_OK;
}

static inline void _UpdateRecord(IndexScan *op, Record r, EntityID node_id) {
	// Get a pointer to a heap allocated node.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Update node's internal entity pointer.
	assert(Graph_GetNode(op->g, node_id, n));
}

static Record IndexScanConsumeFromChild(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		else RediSearch_ResultsIteratorReset(op->iter);
	}

	const EntityID *nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL);
	if(!nodeId) { // Index scan depleted.
		Record_Free(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		RediSearch_ResultsIteratorReset(op->iter);
		nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL);
		if(!nodeId) return NULL; // Empty iterator, return immediately.
	}

	// Clone the held Record, as it will be freed upstream.
	Record r = Record_Clone(op->child_record);

	// Populate the Record with the actual node.
	_UpdateRecord(op, r, *nodeId);

	return r;
}

static Record IndexScanConsume(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	const EntityID *nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL);
	if(!nodeId) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);

	// Populate the Record with the actual node.
	_UpdateRecord(op, r, *nodeId);

	return r;
}

static OpResult IndexScanReset(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	RediSearch_ResultsIteratorReset(op->iter);
	return OP_OK;
}

static void IndexScanFree(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	/* As long as this Index iterator is alive the index is
	 * read locked, if this index scan operation is part of
	 * a query which will modified this index we'll be stuck in
	 * a dead lock, as we're unable to acquire index write lock. */
	if(op->iter) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	}

	if(op->child_record) {
		Record_Free(op->child_record);
		op->child_record = NULL;
	}
}

