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

/* Upon every call to create or reset iterator, there is need to see if the matrix size might have changed.
 * The upper bound of the range should be a result of comparing the INPUT upper bound, and the maximal number of rows in matrix.*/
static inline void _setIndices(NodeByLabelScan *op) {
	// The largest possible entity ID is the same as Graph_RequiredMatrixDim.
	NodeID max_id;
	if(op->id_range) max_id = op->id_range->include_max ? op->id_range->max : op->id_range->max - 1;
	else max_id = op->maxId;
	op->maxId = MIN(max_id,  Graph_RequiredMatrixDim(op->g) - 1);
}

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, const QGNode *n) {
	NodeByLabelScan *op = malloc(sizeof(NodeByLabelScan));
	GraphContext *gc = QueryCtx_GetGraphCtx();
	op->g = gc->g;
	op->n = n;
	op->iter = NULL;
	op->child_record = NULL;
	op->id_range = NULL;

	// The smallest possible entity ID is 0.
	op->minId = 0;
	op->maxId = UINT64_MAX;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan", NodeByLabelScanInit,
				NodeByLabelScanConsume, NodeByLabelScanReset, NodeByLabelScanToString, NodeByLabelScanFree, false,
				plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);

	return (OpBase *)op;
}

void NodeByLabelScanOp_SetIDRange(NodeByLabelScan *op, UnsignedRange *id_range) {
	op->id_range = UnsignedRange_New();
	memcpy(op->id_range, id_range, sizeof(UnsignedRange));
	op->minId = op->id_range->include_min ? op->id_range->min : op->id_range->min + 1;
	op->op.type = OpType_NODE_BY_LABEL_AND_ID_SCAN;
	op->op.name = "Node By Label and ID Scan";
}

static void _ConstructIterator(NodeByLabelScan *op) {
	_setIndices(op);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
	GxB_MatrixTupleIter_new(&op->iter, Graph_GetLabelMatrix(gc->g, schema->id));
	GxB_MatrixTupleIter_iterate_range(op->iter, op->minId, op->maxId);
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
	_setIndices(op);
	GxB_MatrixTupleIter_iterate_range(op->iter, op->minId, op->maxId);
}

static Record NodeByLabelScanConsumeFromChild(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		// One-time construction of the iterator.
		// (Don't do so before now because the label might have been built in a child op.)
		if(op->iter) {
			_ResetIterator(op);
		} else {
			// Iterator wasn't set up until now
			GraphContext *gc = QueryCtx_GetGraphCtx();
			Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
			// No label matrix, no need to iterate - consume child.
			if(!schema) {
				while(OpBase_Consume(op->op.children[0])) {}
				Record_Free(op->child_record); // Free old record.
				op->child_record = NULL;
				return NULL;
			} else {
				_ConstructIterator((NodeByLabelScan *)opBase);
			}
		}
	}

	GrB_Index nodeId;
	bool depleted = false;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	while(depleted) {
		Record_Free(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		_ResetIterator(op);
		GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
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

/* This function is invoked when the op has no children and no valid label is requested (either no label, or non existing label).
 * The op simply needs to return NULL */
static Record NodeByLabelScanNoOp(OpBase *opBase) {
	return NULL;
}

static OpResult NodeByLabelScanReset(OpBase *ctx) {
	NodeByLabelScan *op = (NodeByLabelScan *)ctx;
	if(op->child_record) {
		Record_Free(op->child_record); // Free old record.
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
		Record_Free(nodeByLabelScan->child_record);
		nodeByLabelScan->child_record = NULL;
	}

	if(nodeByLabelScan->id_range) {
		UnsignedRange_Free(nodeByLabelScan->id_range);
		nodeByLabelScan->id_range = NULL;
	}
}
