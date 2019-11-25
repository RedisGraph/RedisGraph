/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_label_scan.h"
#include "../../ast/ast.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult NodeByLabelScanInit(OpBase *opBase);
static Record NodeByLabelScanConsume(OpBase *opBase);
static Record NodeByLabelScanConsumeFromChild(OpBase *opBase);
static OpResult NodeByLabelScanReset(OpBase *opBase);
static void NodeByLabelScanFree(OpBase *opBase);

static int NodeByLabelScanToString(const OpBase *ctx, char *buff, uint buff_len) {
	const NodeByLabelScan *op = (const NodeByLabelScan *)ctx;
	int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
	offset += QGNode_ToString(op->n, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, const QGNode *node) {
	NodeByLabelScan *op = malloc(sizeof(NodeByLabelScan));
	GraphContext *gc = QueryCtx_GetGraphCtx();
	op->g = gc->g;
	op->n = node;
	op->iter = NULL;
	op->_zero_matrix = NULL;
	op->child_record = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan", NodeByLabelScanInit,
				NodeByLabelScanConsume, NodeByLabelScanReset, NodeByLabelScanToString, NodeByLabelScanFree, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, node->alias);

	return (OpBase *)op;
}

static inline void _ConstructIterator(NodeByLabelScan *op) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
	if(schema) {
		GxB_MatrixTupleIter_new(&op->iter, Graph_GetLabelMatrix(gc->g, schema->id));
	} else {
		/* Label does not exist, use a fake empty matrix. */
		GrB_Matrix_new(&op->_zero_matrix, GrB_BOOL, 1, 1);
		GxB_MatrixTupleIter_new(&op->iter, op->_zero_matrix);
	}
}

static OpResult NodeByLabelScanInit(OpBase *opBase) {
	if(opBase->childCount > 0) {
		opBase->consume = NodeByLabelScanConsumeFromChild;
	} else {
		// If we have no children, we can build the iterator now.
		_ConstructIterator((NodeByLabelScan *)opBase);
	}

	return OP_OK;
}

static inline void _UpdateRecord(NodeByLabelScan *op, Record r, GrB_Index node_id) {
	// Get a pointer to the node's allocated space within the Record.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Populate the Record with the graph entity data.
	Graph_GetNode(op->g, node_id, n);
}

static Record NodeByLabelScanConsumeFromChild(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		// One-time construction of the iterator.
		// (Don't do so before now because the label might have been built in a child op.)
		if(op->iter == NULL) _ConstructIterator(op);
		else GxB_MatrixTupleIter_reset(op->iter);
	}

	GrB_Index nodeId;
	bool depleted = false;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	if(depleted) {
		Record_Free(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
		if(depleted) return NULL; // Empty iterator; return immediately.
	}

	// Clone the held Record, as it will be freed upstream.
	Record r = Record_Clone(op->child_record);

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

static OpResult NodeByLabelScanReset(OpBase *ctx) {
	NodeByLabelScan *op = (NodeByLabelScan *)ctx;
	GxB_MatrixTupleIter_reset(op->iter);
	return OP_OK;
}

static void NodeByLabelScanFree(OpBase *op) {
	NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan *)op;

	if(nodeByLabelScan->iter) {
		GxB_MatrixTupleIter_free(nodeByLabelScan->iter);
		nodeByLabelScan->iter = NULL;
	}

	if(nodeByLabelScan->_zero_matrix) {
		GrB_Matrix_free(&nodeByLabelScan->_zero_matrix);
		nodeByLabelScan->_zero_matrix = NULL;
	}

	if(nodeByLabelScan->child_record) {
		Record_Free(nodeByLabelScan->child_record);
		nodeByLabelScan->child_record = NULL;
	}
}

