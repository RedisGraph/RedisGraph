/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_index_scan.h"

int IndexScanToString(const OpBase *ctx, char *buff, uint buff_len) {
	const IndexScan *op = (const IndexScan *)ctx;
	int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
	offset += QGNode_ToString(op->n, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewIndexScanOp(Graph *g, QGNode *n, RSIndex *idx, RSResultsIterator *iter) {
	IndexScan *indexScan = malloc(sizeof(IndexScan));
	indexScan->g = g;
	indexScan->n = n;
	indexScan->idx = idx;
	indexScan->iter = iter;
	indexScan->nodeRecIdx = -1;

	// Set our Op operations
	OpBase_Init(&indexScan->op);
	indexScan->op.name = "Index Scan";
	indexScan->op.type = OPType_INDEX_SCAN;
	indexScan->op.reset = IndexScanReset;
	indexScan->op.consume = IndexScanConsume;
	indexScan->op.toString = IndexScanToString;
	indexScan->op.free = IndexScanFree;

	OpBase_Modifies(indexScan, n->alias);

	return (OpBase *)indexScan;
}

Record IndexScanConsume(OpBase *opBase) {
	IndexScan *op = (IndexScan *)opBase;
	const EntityID *nodeId = RediSearch_ResultsIteratorNext(op->iter, op->idx, NULL);
	if(!nodeId) return NULL;

	Record r = Record_New(opBase->record_map->record_len);
	// Get a pointer to a heap allocated node.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Update node's internal entity pointer.
	Graph_GetNode(op->g, *nodeId, n);

	return r;
}

OpResult IndexScanReset(OpBase *ctx) {
	IndexScan *op = (IndexScan *)ctx;
	RediSearch_ResultsIteratorReset(op->iter);
	return OP_OK;
}

void IndexScanFree(OpBase *ctx) {
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
