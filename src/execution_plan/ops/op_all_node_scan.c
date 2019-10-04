/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../ast/ast.h"

/* Forward declarations. */
static Record AllNodeScanConsume(OpBase *opBase);
static OpResult AllNodeScanReset(OpBase *opBase);
static void AllNodeScanFree(OpBase *opBase);

static int AllNodeScanToString(const OpBase *ctx, char *buff, uint buff_len) {
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
	OpBase_Init((OpBase *)op, OPType_ALL_NODE_SCAN, "All Node Scan", NULL,
				AllNodeScanConsume, AllNodeScanReset, AllNodeScanToString, AllNodeScanFree, plan);
	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);
	return (OpBase *)op;
}

static Record AllNodeScanConsume(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;

	Entity *en = (Entity *)DataBlockIterator_Next(op->iter);
	if(en == NULL) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	n->entity = en;

	return r;
}

static OpResult AllNodeScanReset(OpBase *op) {
	AllNodeScan *allNodeScan = (AllNodeScan *)op;
	DataBlockIterator_Reset(allNodeScan->iter);
	return OP_OK;
}

static void AllNodeScanFree(OpBase *ctx) {
	AllNodeScan *op = (AllNodeScan *)ctx;
	if(op->iter) {
		DataBlockIterator_Free(op->iter);
		op->iter = NULL;
	}
}

