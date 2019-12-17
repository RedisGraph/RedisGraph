/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_label_scan.h"
#include "op_node_by_id_seek.h"
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

// Checks to see if operation index is within its bounds.
static inline bool _outOfBounds(NodeByLabelScan *op, NodeID id) {
	/* Because currentId starts at minimum and only increases
	 * we only care about top bound. */
	if(id > op->maxId) return true;
	if(id == op->maxId && !op->maxInclusive) return true;
	return false;
}

static inline void _setIndices(NodeByLabelScan *op) {
	// The largest possible entity ID is the same as Graph_RequiredMatrixDim.
	op->maxId = MIN(op->maxId, Graph_RequiredMatrixDim(op->g));
}

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, const QGNode *n) {
	NodeByLabelScan *op = malloc(sizeof(NodeByLabelScan));
	GraphContext *gc = QueryCtx_GetGraphCtx();
	op->g = gc->g;
	op->n = n;
	op->iter = NULL;
	op->child_record = NULL;

	// The smallest possible entity ID is 0.
	op->minId = 0;
	op->minInclusive = true;
	// Set now, final set upon iterator creation.
	op->maxId = ID_RANGE_UNBOUND;
	op->maxInclusive = false;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan", NodeByLabelScanInit,
				NodeByLabelScanConsume, NodeByLabelScanReset, NodeByLabelScanToString, NodeByLabelScanFree, false,
				plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n->alias);

	return (OpBase *)op;
}

void NodeByLabelScanOp_IDRange(NodeByLabelScan *op, NodeID minId, bool minInclusive, NodeID maxId,
							   bool maxInclusive) {
	// Can't include unspecified bound.
	assert(!(minId == ID_RANGE_UNBOUND && minInclusive));
	assert(!(maxId == ID_RANGE_UNBOUND && maxInclusive));
	if(minId == ID_RANGE_UNBOUND && !minInclusive) {
		minId = 0;
		minInclusive = true;
	}

	op->minId = minId;
	op->minInclusive = minInclusive;
	op->maxId = maxId;
	op->maxInclusive = maxInclusive;
	op->op.type = OpType_NODE_BY_LABEL_AND_ID_SCAN;
	op->op.name = "Node By Label and ID Scan";
}

static inline bool _ConstructIterator(NodeByLabelScan *op) {
	_setIndices(op);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *schema = GraphContext_GetSchema(gc, op->n->label, SCHEMA_NODE);
	if(schema) {
		GxB_MatrixTupleIter_new(&op->iter, Graph_GetLabelMatrix(gc->g, schema->id));
		NodeID start_row = op->minInclusive ? op->minId : op->minId + 1;
		GxB_MatrixTupleIter_jump_to_row(op->iter, start_row);
		return true;
	} else {
		/* Label does not exist, no need to iterate. */
		return false;
	}
}

static OpResult NodeByLabelScanInit(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;
	if(opBase->childCount > 0) {
		opBase->consume = NodeByLabelScanConsumeFromChild;
	} else {
		// If we have no children, we can build the iterator now.
		if(!_ConstructIterator((NodeByLabelScan *)opBase)) {
			// No label matrix, just return null.
			opBase->consume = NodeByLabelScanNoOp;
		}
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
	NodeID start_row = op->minInclusive ? op->minId : op->minId + 1;
	GxB_MatrixTupleIter_jump_to_row(op->iter, start_row);
}

static Record NodeByLabelScanConsumeFromChild(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		// One-time construction of the iterator.
		// (Don't do so before now because the label might have been built in a child op.)
		if(op->iter == NULL) {
			if(!_ConstructIterator(op)) {
				// No need to iterate - consume child.
				while(OpBase_Consume(op->op.children[0])) {}
				return NULL;
			}
		} else {
			_ResetIterator(op);
		}
	}

	GrB_Index nodeId;
	bool depleted = false;
	GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
	if(depleted || _outOfBounds(op, nodeId)) {
		Record_Free(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		_ResetIterator(op);
		GxB_MatrixTupleIter_next(op->iter, NULL, &nodeId, &depleted);
		if(depleted || _outOfBounds(op, nodeId)) return NULL; // Empty iterator; return immediately.
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
	if(depleted || _outOfBounds(op, nodeId)) return NULL;

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
}
