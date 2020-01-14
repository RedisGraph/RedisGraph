/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_label_scan.h"
#include "shared/print_functions.h"
#include "../../ast/ast.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult NodeByLabelScanInit(OpBase *opBase);
static Record NodeByLabelScanConsume(OpBase *opBase);
static Record NodeByLabelScanConsumeFromChild(OpBase *opBase);
static Record NodeByLabelScanNoOp(OpBase *opBase);
static OpResult NodeByLabelScanReset(OpBase *opBase);
static void NodeByLabelScanFree(OpBase *opBase);

static inline int NodeByLabelScanToString(const OpBase *ctx, char *buf, uint buf_len) {
	return ScanToString(ctx, buf, buf_len, ((const NodeByLabelScan *)ctx)->n);
}

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, const QGNode *n) {
	NodeByLabelScan *op = rm_malloc(sizeof(NodeByLabelScan));
	GraphContext *gc = QueryCtx_GetGraphCtx();
	op->g = gc->g;
	op->n = n;
	op->iter = NULL;
	op->child_record = NULL;
	// Defaults to [0...UINT64_MAX].
	op->id_range = UnsignedRange_New();

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan", NodeByLabelScanInit,
				NodeByLabelScanConsume, NodeByLabelScanReset, NodeByLabelScanToString, NodeByLabelScanFree, false,
				plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);

	return (OpBase *)op;
}

void NodeByLabelScanOp_SetIDRange(NodeByLabelScan *op, UnsignedRange *id_range) {
	memcpy(op->id_range, id_range, sizeof(UnsignedRange));
	op->op.type = OpType_NODE_BY_LABEL_AND_ID_SCAN;
	op->op.name = "Node By Label and ID Scan";
}

static void _ConstructIterator(NodeByLabelScan *op) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
	assert(schema);
	GxB_MatrixTupleIter_new(&op->iter, Graph_GetLabelMatrix(gc->g, schema->id));
	NodeID minId = op->id_range->include_min ? op->id_range->min : op->id_range->min + 1;
	NodeID maxId = op->id_range->include_max ? op->id_range->max : op->id_range->max - 1 ;
	GxB_MatrixTupleIter_iterate_range(op->iter, minId, maxId);

}

static OpResult NodeByLabelScanInit(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;
	if(opBase->childCount > 0) {
		opBase->consume = NodeByLabelScanConsumeFromChild;
	} else {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
		// No label matrix, just return null.
		if(!schema) opBase->consume = NodeByLabelScanNoOp;
		// If we have no children, we can build the iterator now.
		else _ConstructIterator((NodeByLabelScan *)opBase);
	}

	return OP_OK;
}

static inline void _UpdateRecord(NodeByLabelScan *op, Record r, GrB_Index node_id) {
	// Get a pointer to the node's allocated space within the Record.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Populate the Record with the graph entity data.
	Graph_GetNode(op->g, node_id, n);
}

static inline void _ResetIterator(NodeByLabelScan *op) {
	NodeID minId = op->id_range->include_min ? op->id_range->min : op->id_range->min + 1;
	NodeID maxId = op->id_range->include_max ? op->id_range->max : op->id_range->max - 1 ;
	GxB_MatrixTupleIter_iterate_range(op->iter, minId, maxId);
}

static Record NodeByLabelScanConsumeFromChild(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	// Try to get new nodeID.
	GrB_Index nodeId;
	bool depleted = true;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	/* depleted will be true in the following cases:
	 * 1. No iterator: GxB_MatrixTupleIter_next will fail and depleted will stay true. This scenario means
	 * that there was no consumption of a record from a child, otherwise there was an iterator.
	 * 2. Iterator depleted - For every child record the iterator finished the entire matrix scan and it needs to restart. */
	while(depleted) {
		// Try to get a record.
		if(op->child_record) OpBase_DeleteRecord(op->child_record);
		op->child_record = NULL;
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;

		// Got a record.
		if(!op->iter) {
			// Iterator wasn't set up until now.
			GraphContext *gc = QueryCtx_GetGraphCtx();
			Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
			// No label matrix, it might be created in the next iteration.
			if(!schema) continue;
			_ConstructIterator((NodeByLabelScan *)opBase);

		} else {
			// Iterator depleted - reset.
			_ResetIterator(op);
		}
		// Try to get new NodeID.
		GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	}

	// We've got a record and NodeID.
	// Clone the held Record, as it will be freed upstream.
	Record r = OpBase_CloneRecord(op->child_record);
	// Populate the Record with the actual node.
	_UpdateRecord(op, r, nodeId);
	return r;
}

static Record NodeByLabelScanConsume(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	GrB_Index nodeId;
	bool depleted = false;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	if(depleted) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);

	// Populate the Record with the actual node.
	_UpdateRecord(op, r, nodeId);

	return r;
}

/* This function is invoked when the op has no children and no valid label is requested (either no label, or non existing label).
 * The op simply needs to return NULL */
static Record NodeByLabelScanNoOp(OpBase *opBase) {
	return NULL;
}

static OpResult NodeByLabelScanReset(OpBase *ctx) {
	NodeByLabelScan *op = (NodeByLabelScan *)ctx;
	if(op->child_record) {
		OpBase_DeleteRecord(op->child_record); // Free old record.
		op->child_record = NULL;
	}
	_ResetIterator(op);
	return OP_OK;
}

static void NodeByLabelScanFree(OpBase *op) {
	NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan *)op;

	if(nodeByLabelScan->iter) {
		GxB_MatrixTupleIter_free(nodeByLabelScan->iter);
		nodeByLabelScan->iter = NULL;
	}

	if(nodeByLabelScan->child_record) {
		OpBase_DeleteRecord(nodeByLabelScan->child_record);
		nodeByLabelScan->child_record = NULL;
	}

	if(nodeByLabelScan->id_range) {
		UnsignedRange_Free(nodeByLabelScan->id_range);
		nodeByLabelScan->id_range = NULL;
	}
}
