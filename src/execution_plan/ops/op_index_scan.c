/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_index_scan.h"

/* Forward declarations. */
static Record IndexScanConsume(OpBase *opBase);
static OpResult IndexScanReset(OpBase *opBase);
static void IndexScanFree(OpBase *opBase);

static int ToString(const OpBase *ctx, char *buff, uint buff_len) {
	const IndexScan *op = (const IndexScan *)ctx;
	int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
	offset += QGNode_ToString(op->n, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewIndexScanOp(const ExecutionPlan *plan, Graph *g, const QGNode *n, RSIndex *idx,
					   RSResultsIterator *iter) {
	IndexScan *op = malloc(sizeof(IndexScan));
	op->g = g;
	op->n = n;
	op->idx = idx;
	op->iter = iter;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_INDEX_SCAN, "Index Scan", NULL, IndexScanConsume, IndexScanReset,
				ToString, IndexScanFree, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);
	return (OpBase *)op;
}

static Record IndexScanConsume(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	const EntityID *nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL);
	if(!nodeId) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	// Get a pointer to a heap allocated node.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Update node's internal entity pointer.
	Graph_GetNode(op->g, *nodeId, n);

	return r;
}

static OpResult IndexScanReset(OpBase *ctx) {
	IndexScan *op = (IndexScan *)ctx;
	RediSearch_ResultsIteratorReset(op->iter);
	return OP_OK;
}

static void IndexScanFree(OpBase *ctx) {
	IndexScan *op = (IndexScan *)ctx;
	/* As long as this Index iterator is alive the index is
	 * read locked, if this index scan operation is part of
	 * a query which will modified this index we'll be stuck in
	 * a dead lock, as we're unable to acquire index write lock. */
	if(op->iter) {
		RediSearch_ResultsIteratorFree(op->iter);
		op->iter = NULL;
	}
}

