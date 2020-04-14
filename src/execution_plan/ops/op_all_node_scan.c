/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "../../query_ctx.h"
#include "shared/print_functions.h"

/* Forward declarations. */
static OpResult AllNodeScanInit(OpBase *opBase);
static Record AllNodeScanConsume(OpBase *opBase);
static Record AllNodeScanConsumeFromChild(OpBase *opBase);
static OpResult AllNodeScanReset(OpBase *opBase);
static OpBase *AllNodeScanClone(const ExecutionPlan *plan, const OpBase *opBase);
static void AllNodeScanFree(OpBase *opBase);

static inline int AllNodeScanToString(const OpBase *ctx, char *buf, uint buf_len) {
	return ScanToString(ctx, buf, buf_len, ((const AllNodeScan *)ctx)->n);
}

OpBase *NewAllNodeScanOp(const ExecutionPlan *plan, const QGNode *n) {
	AllNodeScan *op = rm_malloc(sizeof(AllNodeScan));
	op->n = n;
	op->iter = NULL;
	op->child_record = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_ALL_NODE_SCAN, "All Node Scan", AllNodeScanInit,
				AllNodeScanConsume, AllNodeScanReset, AllNodeScanToString, AllNodeScanClone, AllNodeScanFree, false,
				plan);
	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);
	return (OpBase *)op;
}

static OpResult AllNodeScanInit(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;
	if(opBase->childCount > 0) OpBase_UpdateConsume(opBase, AllNodeScanConsumeFromChild);
	else op->iter = Graph_ScanNodes(QueryCtx_GetGraph());
	return OP_OK;
}

static Record AllNodeScanConsumeFromChild(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		else {
			if(!op->iter) op->iter = Graph_ScanNodes(QueryCtx_GetGraph());
			else DataBlockIterator_Reset(op->iter);
		}
	}

	Entity *en = (Entity *)DataBlockIterator_Next(op->iter);
	if(en == NULL) {
		OpBase_DeleteRecord(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		DataBlockIterator_Reset(op->iter);
		en = DataBlockIterator_Next(op->iter);
		if(!en) return NULL; // Iterator was empty; return immediately.
	}

	// Clone the held Record, as it will be freed upstream.
	Record r = OpBase_CloneRecord(op->child_record);

	// Populate the Record with the graph entity data.
	Node n = { .entity = en };
	Record_AddNode(r, op->nodeRecIdx, n);

	return r;
}

static Record AllNodeScanConsume(OpBase *opBase) {
	AllNodeScan *op = (AllNodeScan *)opBase;

	Entity *en = (Entity *)DataBlockIterator_Next(op->iter);
	if(en == NULL) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	Node n = { .entity = en };
	Record_AddNode(r, op->nodeRecIdx, n);

	return r;
}

static OpResult AllNodeScanReset(OpBase *op) {
	AllNodeScan *allNodeScan = (AllNodeScan *)op;
	if(allNodeScan->iter) DataBlockIterator_Reset(allNodeScan->iter);
	return OP_OK;
}

static inline OpBase *AllNodeScanClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_ALL_NODE_SCAN);
	AllNodeScan *allNodeScan = (AllNodeScan *)opBase;
	return NewAllNodeScanOp(plan, allNodeScan->n);
}

static void AllNodeScanFree(OpBase *ctx) {
	AllNodeScan *op = (AllNodeScan *)ctx;
	if(op->iter) {
		DataBlockIterator_Free(op->iter);
		op->iter = NULL;
	}

	if(op->child_record) {
		OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
	}
}

