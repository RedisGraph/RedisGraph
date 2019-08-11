/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_label_scan.h"
#include "../../ast/ast.h"

int NodeByLabelScanToString(const OpBase *ctx, char *buff, uint buff_len) {
	const NodeByLabelScan *op = (const NodeByLabelScan *)ctx;
	int offset = snprintf(buff, buff_len, "%s | ", op->op.name);
	offset += QGNode_ToString(op->node, buff + offset, buff_len - offset);
	return offset;
}

OpBase *NewNodeByLabelScanOp(QGNode *node) {
	NodeByLabelScan *nodeByLabelScan = malloc(sizeof(NodeByLabelScan));
	GraphContext *gc = GraphContext_GetFromTLS();
	nodeByLabelScan->g = gc->g;
	nodeByLabelScan->node = node;
	nodeByLabelScan->_zero_matrix = NULL;
	nodeByLabelScan->nodeRecIdx = -1;

	/* Find out label matrix ID. */
	Schema *schema = GraphContext_GetSchema(gc, node->label, SCHEMA_NODE);
	if(schema) {
		GxB_MatrixTupleIter_new(&nodeByLabelScan->iter, Graph_GetLabelMatrix(gc->g, schema->id));
	} else {
		/* Label does not exist, use a fake empty matrix. */
		GrB_Matrix_new(&nodeByLabelScan->_zero_matrix, GrB_BOOL, 1, 1);
		GxB_MatrixTupleIter_new(&nodeByLabelScan->iter, nodeByLabelScan->_zero_matrix);
	}

	// Set our Op operations
	OpBase_Init(&nodeByLabelScan->op);
	nodeByLabelScan->op.name = "Node By Label Scan";
	nodeByLabelScan->op.type = OPType_NODE_BY_LABEL_SCAN;
	nodeByLabelScan->op.consume = NodeByLabelScanConsume;
	nodeByLabelScan->op.init = NodeByLabelScanInit;
	nodeByLabelScan->op.reset = NodeByLabelScanReset;
	nodeByLabelScan->op.toString = NodeByLabelScanToString;
	nodeByLabelScan->op.free = NodeByLabelScanFree;

	OpBase_Modifies((OpBase *)nodeByLabelScan, node->alias);

	return (OpBase *)nodeByLabelScan;
}

OpResult NodeByLabelScanInit(OpBase *opBase) {
	return OP_OK;
}

Record NodeByLabelScanConsume(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	GrB_Index nodeId;
	bool depleted = false;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	if(depleted) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	if(op->nodeRecIdx == -1) {
		op->nodeRecIdx = Record_GetEntryIdx(r, op->node->alias);
	}

	// Get a pointer to a heap allocated node.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Update node's internal entity pointer.
	Graph_GetNode(op->g, nodeId, n);
	return r;
}

OpResult NodeByLabelScanReset(OpBase *ctx) {
	NodeByLabelScan *op = (NodeByLabelScan *)ctx;
	GxB_MatrixTupleIter_reset(op->iter);
	return OP_OK;
}

void NodeByLabelScanFree(OpBase *op) {
	NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan *)op;

	if(nodeByLabelScan->iter) {
		GxB_MatrixTupleIter_free(nodeByLabelScan->iter);
		nodeByLabelScan->iter = NULL;
	}

	if(nodeByLabelScan->_zero_matrix) {
		GrB_Matrix_free(&nodeByLabelScan->_zero_matrix);
		nodeByLabelScan->_zero_matrix = NULL;
	}
}
