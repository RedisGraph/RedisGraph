/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../ast/ast.h"

/* Forward declarations. */
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

static int ToString(const OpBase *ctx, char *buff, uint buff_len) {
	const AllNodeScan *op = (const AllNodeScan *)ctx;
	int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
	offset += QGNode_ToString(op->n, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const Graph *g, const QGNode *n) {
	AllNodeScan *op = malloc(sizeof(AllNodeScan));
	op->n = n;
	op->iter = Graph_ScanNodes(g);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ALL_NODE_SCAN, "All Node Scan", Init, Consume, Reset, ToString,
				Free, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);
	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;

	Entity *en = (Entity *)DataBlockIterator_Next(op->iter);
	if(en == NULL) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	n->entity = en;

	return r;
}

static OpResult Reset(OpBase *op) {
	AllNodeScan *allNodeScan = (AllNodeScan *)op;
	DataBlockIterator_Reset(allNodeScan->iter);
	return OP_OK;
}

static void Free(OpBase *ctx) {
	AllNodeScan *op = (AllNodeScan *)ctx;
	if(op->iter) {
		DataBlockIterator_Free(op->iter);
		op->iter = NULL;
	}
}
