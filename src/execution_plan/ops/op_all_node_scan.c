/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../ast/ast.h"

int AllNodeScanToString(const OpBase *ctx, char *buff, uint buff_len) {
	const AllNodeScan *op = (const AllNodeScan *)ctx;
	int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
	offset += QGNode_ToString(op->n, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewAllNodeScanOp(const Graph *g, QGNode *n) {
	AllNodeScan *allNodeScan = malloc(sizeof(AllNodeScan));
	allNodeScan->n = n;
	allNodeScan->iter = Graph_ScanNodes(g);
	allNodeScan->nodeRecIdx = -1;

	// Set our Op operations
	OpBase_Init(&allNodeScan->op);
	allNodeScan->op.name = "All Node Scan";
	allNodeScan->op.type = OPType_ALL_NODE_SCAN;
	allNodeScan->op.consume = AllNodeScanConsume;
	allNodeScan->op.init = AllNodeScanInit;
	allNodeScan->op.reset = AllNodeScanReset;
	allNodeScan->op.toString = AllNodeScanToString;
	allNodeScan->op.free = AllNodeScanFree;

	OpBase_Modifies((OpBase *)allNodeScan, n->alias);

	return (OpBase *)allNodeScan;
}

OpResult AllNodeScanInit(OpBase *opBase) {
	return OP_OK;
}

Record AllNodeScanConsume(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;

	Entity *en = (Entity *)DataBlockIterator_Next(op->iter);
	if(en == NULL) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	if(op->nodeRecIdx == -1) {
		op->nodeRecIdx = Record_GetEntryIdx(r, op->n->alias);
	}
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	n->entity = en;

	return r;
}

OpResult AllNodeScanReset(OpBase *op) {
	AllNodeScan *allNodeScan = (AllNodeScan *)op;
	DataBlockIterator_Reset(allNodeScan->iter);
	return OP_OK;
}

void AllNodeScanFree(OpBase *ctx) {
	AllNodeScan *op = (AllNodeScan *)ctx;
	if(op->iter) {
		DataBlockIterator_Free(op->iter);
		op->iter = NULL;
	}
}
