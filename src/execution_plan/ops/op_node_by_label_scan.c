/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_label_scan.h"
#include "../../ast/ast.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static Record NodeByLabelScanConsume(OpBase *opBase);
static OpResult NodeByLabelScanReset(OpBase *opBase);
static void NodeByLabelScanFree(OpBase *opBase);

static int ToString(const OpBase *ctx, char *buff, uint buff_len) {
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
	op->_zero_matrix = NULL;

	/* Find out label matrix ID. */
	Schema *schema = GraphContext_GetSchema(gc, node->label, SCHEMA_NODE);
	if(schema) {
		GxB_MatrixTupleIter_new(&op->iter, Graph_GetLabelMatrix(gc->g, schema->id));
	} else {
		/* Label does not exist, use a fake empty matrix. */
		GrB_Matrix_new(&op->_zero_matrix, GrB_BOOL, 1, 1);
		GxB_MatrixTupleIter_new(&op->iter, op->_zero_matrix);
	}

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan", NULL,
				NodeByLabelScanConsume, NodeByLabelScanReset, ToString, NodeByLabelScanFree, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, node->alias);

	return (OpBase *)op;
}

static Record NodeByLabelScanConsume(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	GrB_Index nodeId;
	bool depleted = false;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	if(depleted) return NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	// Get a pointer to a heap allocated node.
	Node *n = Record_GetNode(r, op->nodeRecIdx);
	// Update node's internal entity pointer.
	Graph_GetNode(op->g, nodeId, n);
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
}

