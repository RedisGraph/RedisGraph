/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_all_node_scan.h"
#include "query_ctx.h"

/* Forward declarations. */
static RT_OpResult AllNodeScanInit(RT_OpBase *opBase);
static Record AllNodeScanConsume(RT_OpBase *opBase);
static Record AllNodeScanConsumeFromChild(RT_OpBase *opBase);
static RT_OpResult AllNodeScanReset(RT_OpBase *opBase);
static RT_OpBase *AllNodeScanClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void AllNodeScanFree(RT_OpBase *opBase);

RT_OpBase *RT_NewAllNodeScanOp(const RT_ExecutionPlan *plan, const char *alias) {
	RT_AllNodeScan *op = rm_malloc(sizeof(RT_AllNodeScan));
	op->iter = NULL;
	op->alias = alias;
	op->child_record = NULL;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_ALL_NODE_SCAN, AllNodeScanInit,
				AllNodeScanConsume, AllNodeScanReset, AllNodeScanClone, AllNodeScanFree, false,
				plan);
	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, alias);
	return (RT_OpBase *)op;
}

static RT_OpResult AllNodeScanInit(RT_OpBase *opBase) {
	RT_AllNodeScan *op = (RT_AllNodeScan *)opBase;
	if(opBase->childCount > 0) RT_OpBase_UpdateConsume(opBase, AllNodeScanConsumeFromChild);
	else op->iter = Graph_ScanNodes(QueryCtx_GetGraph());
	return OP_OK;
}

static Record AllNodeScanConsumeFromChild(RT_OpBase *opBase) {
	RT_AllNodeScan *op = (RT_AllNodeScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = RT_OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		else {
			if(!op->iter) op->iter = Graph_ScanNodes(QueryCtx_GetGraph());
			else DataBlockIterator_Reset(op->iter);
		}
	}

	Node n = GE_NEW_NODE();
	n.entity = (Entity *)DataBlockIterator_Next(op->iter, &n.id);
	if(n.entity == NULL) {
		RT_OpBase_DeleteRecord(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = RT_OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		DataBlockIterator_Reset(op->iter);
		n.entity = DataBlockIterator_Next(op->iter, &n.id);
		if(n.entity == NULL) return NULL; // Iterator was empty; return immediately.
	}

	// Clone the held Record, as it will be freed upstream.
	Record r = RT_OpBase_CloneRecord(op->child_record);

	// Populate the Record with the graph entity data.
	Record_AddNode(r, op->nodeRecIdx, n);

	return r;
}

static Record AllNodeScanConsume(RT_OpBase *opBase) {
	RT_AllNodeScan *op = (RT_AllNodeScan *)opBase;

	Node n = GE_NEW_NODE();
	n.entity = (Entity *)DataBlockIterator_Next(op->iter, &n.id);
	if(n.entity == NULL) return NULL;

	Record r = RT_OpBase_CreateRecord((RT_OpBase *)op);
	Record_AddNode(r, op->nodeRecIdx, n);

	return r;
}

static RT_OpResult AllNodeScanReset(RT_OpBase *op) {
	RT_AllNodeScan *allNodeScan = (RT_AllNodeScan *)op;
	if(allNodeScan->iter) DataBlockIterator_Reset(allNodeScan->iter);
	return OP_OK;
}

static inline RT_OpBase *AllNodeScanClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_ALL_NODE_SCAN);
	return RT_NewAllNodeScanOp(plan, ((RT_AllNodeScan *)opBase)->alias);
}

static void AllNodeScanFree(RT_OpBase *ctx) {
	RT_AllNodeScan *op = (RT_AllNodeScan *)ctx;
	if(op->iter) {
		DataBlockIterator_Free(op->iter);
		op->iter = NULL;
	}

	if(op->child_record) {
		RT_OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
	}
}
