/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "shared/print_functions.h"

/* Forward declarations. */
static OpResult AllNodeScanInit(OpBase *opBase);
static Record AllNodeScanConsume(OpBase *opBase);
static Record AllNodeScanConsumeFromChild(OpBase *opBase);
static OpResult AllNodeScanReset(OpBase *opBase);
static void AllNodeScanFree(OpBase *opBase);

static inline int AllNodeScanToString(const OpBase *ctx, char *buf, uint buf_len) {
	return ScanToString(ctx, buf, buf_len, ((const AllNodeScan *)ctx)->n);
}

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const Graph *g, const QGNode *n) {
	AllNodeScan *op = malloc(sizeof(AllNodeScan));
	op->n = n;
	op->iter = Graph_ScanNodes(g);
	op->child_record = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ALL_NODE_SCAN, "All Node Scan", AllNodeScanInit,
				AllNodeScanConsume, AllNodeScanReset, AllNodeScanToString, AllNodeScanFree, plan);
	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);
	return (OpBase *)op;
}

static OpResult AllNodeScanInit(OpBase *opBase) {
	if(opBase->childCount > 0) opBase->consume = AllNodeScanConsumeFromChild;
	return OP_OK;
}

static Record AllNodeScanConsumeFromChild(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		else DataBlockIterator_Reset(op->iter);
	}

	Entity *en = (Entity *)DataBlockIterator_Next(op->iter);
	if(en == NULL) {
		Record_Free(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		DataBlockIterator_Reset(op->iter);
		en = DataBlockIterator_Next(op->iter);
		if(!en) return NULL; // Iterator was empty; return immediately.
	}

	// Clone the held Record, as it will be freed upstream.
	Record r = Record_Clone(op->child_record);

	// Populate the Record with the graph entity data.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	n->entity = en;

	return r;
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

	if(op->child_record) {
		Record_Free(op->child_record);
		op->child_record = NULL;
	}
}

