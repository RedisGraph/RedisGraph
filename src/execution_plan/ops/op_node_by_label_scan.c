/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_node_by_label_scan.h"
#include "RG.h"
#include "shared/print_functions.h"
#include "../../ast/ast.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult NodeByLabelScanInit(OpBase *opBase);
static Record NodeByLabelScanConsume(OpBase *opBase);
static Record NodeByLabelScanConsumeFromChild(OpBase *opBase);
static Record NodeByLabelScanNoOp(OpBase *opBase);
static OpResult NodeByLabelScanReset(OpBase *opBase);
static OpBase *NodeByLabelScanClone(const ExecutionPlan *plan, const OpBase *opBase);
static void NodeByLabelScanFree(OpBase *opBase);

static inline void NodeByLabelScanToString(const OpBase *ctx, sds *buf) {
	NodeByLabelScan *op = (NodeByLabelScan *)ctx;
	ScanToString(ctx, buf, op->n.alias, op->n.label);
}

// update the label-id of a cached operation, as it may have not 
// been known when the plan was prepared.
static void _update_label_id(NodeByLabelScan *op) {
	if(op->n.label_id != GRAPH_UNKNOWN_LABEL) return;

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *s = GraphContext_GetSchema(gc, op->n.label, SCHEMA_NODE);
	if(s != NULL) op->n.label_id = Schema_GetID(s);
}

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, NodeScanCtx n) {
	NodeByLabelScan *op = rm_calloc(sizeof(NodeByLabelScan), 1);
	op->g = QueryCtx_GetGraph();
	op->n = n;
	// Defaults to [0...UINT64_MAX].
	op->id_range = UnsignedRange_New();
	_update_label_id(op);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_NODE_BY_LABEL_SCAN, "Node By Label Scan", NodeByLabelScanInit,
				NodeByLabelScanConsume, NodeByLabelScanReset, NodeByLabelScanToString, NodeByLabelScanClone,
				NodeByLabelScanFree, false, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, n.alias);

	return (OpBase *)op;
}

void NodeByLabelScanOp_SetIDRange(NodeByLabelScan *op, UnsignedRange *id_range) {
	UnsignedRange_Free(op->id_range);
	op->id_range = UnsignedRange_Clone(id_range);

	op->op.type = OPType_NODE_BY_LABEL_AND_ID_SCAN;
	op->op.name = "Node By Label and ID Scan";
}

static GrB_Info _ConstructIterator(NodeByLabelScan *op) {
	NodeID minId;
	NodeID maxId;
	GrB_Info info;
	GrB_Index nrows;

	GraphContext  *gc  =  QueryCtx_GetGraphCtx();
	RG_Matrix     L    =  Graph_GetLabelMatrix(gc->g, op->n.label_id);

	info = RG_Matrix_nrows(&nrows, L);
	ASSERT(info == GrB_SUCCESS);

	// make sure range is within matrix bounds
	UnsignedRange_TightenRange(op->id_range, OP_GE, 0);
	UnsignedRange_TightenRange(op->id_range, OP_LT, nrows);

	if(!UnsignedRange_IsValid(op->id_range)) return GrB_DIMENSION_MISMATCH;

	if(op->id_range->include_min) minId = op->id_range->min;
	else minId = op->id_range->min + 1;

	if(op->id_range->include_max) maxId = op->id_range->max;
	else maxId = op->id_range->max - 1;

	info = RG_MatrixTupleIter_AttachRange(&op->iter, L, minId, maxId);
	ASSERT(info == GrB_SUCCESS);

	return info;
}

static OpResult NodeByLabelScanInit(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;
	OpBase_UpdateConsume(opBase, NodeByLabelScanConsume); // Default consume function.

	// Operation has children, consume from child.
	if(opBase->childCount > 0) {
		OpBase_UpdateConsume(opBase, NodeByLabelScanConsumeFromChild);
		return OP_OK;
	}

	if(op->n.label_id == GRAPH_UNKNOWN_LABEL) {
		// Missing schema, use the NOP consume function.
		OpBase_UpdateConsume(opBase, NodeByLabelScanNoOp);
		return OP_OK;
	}	

	// The iterator build may fail if the ID range does not match the matrix dimensions.
	GrB_Info iterator_built = _ConstructIterator(op);
	if(iterator_built != GrB_SUCCESS) {
		// Invalid range, use the NOP consume function.
		OpBase_UpdateConsume(opBase, NodeByLabelScanNoOp);
		return OP_OK;
	}

	return OP_OK;
}

static inline void _UpdateRecord(NodeByLabelScan *op, Record r, GrB_Index node_id) {
	// Populate the Record with the graph entity data.
	Node n = GE_NEW_NODE();
	Graph_GetNode(op->g, node_id, &n);
	Record_AddNode(r, op->nodeRecIdx, n);
}

static inline void _ResetIterator(NodeByLabelScan *op) {
	if(op->id_range) {
		// reset the range iterator
		NodeID minId = op->id_range->include_min ? op->id_range->min : op->id_range->min + 1;
		NodeID maxId = op->id_range->include_max ? op->id_range->max : op->id_range->max - 1 ;
		RG_MatrixTupleIter_iterate_range(&op->iter, minId, maxId);
	} else {
		// invalid schema, our consume function is NOP
		op->id_range = UnsignedRange_New();
		GrB_Info iterator_built = _ConstructIterator(op);
		// if the iterator is invalid, our consume function is NOP
		if(iterator_built != GrB_SUCCESS) return;
	}
}

static Record NodeByLabelScanConsumeFromChild(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	// Try to get new nodeID.
	GrB_Index nodeId;
	GrB_Info info = RG_MatrixTupleIter_next_BOOL(&op->iter, &nodeId, NULL, NULL);
	while(info == GrB_NULL_POINTER || op->child_record == NULL || info == GxB_EXHAUSTED) {
		// Try to get a record.
		if(op->child_record) OpBase_DeleteRecord(op->child_record);
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;

		// Got a record.
		if(info == GrB_NULL_POINTER) {
			_update_label_id(op);
			if(_ConstructIterator(op) != GrB_SUCCESS) continue;
		} else {
			// Iterator depleted - reset.
			_ResetIterator(op);
		}
		// Try to get new NodeID.
		info = RG_MatrixTupleIter_next_BOOL(&op->iter, &nodeId, NULL, NULL);
	}

	// We've got a record and NodeID.
	// Clone the held Record, as it will be freed upstream.
	Record r = OpBase_DeepCloneRecord(op->child_record);
	// Populate the Record with the actual node.
	_UpdateRecord(op, r, nodeId);
	return r;
}

static Record NodeByLabelScanConsume(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	GrB_Index nodeId;
	GrB_Info info = RG_MatrixTupleIter_next_BOOL(&op->iter, &nodeId, NULL, NULL);
	if(info == GxB_EXHAUSTED) return NULL;

	ASSERT(info == GrB_SUCCESS);

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

static OpBase *NodeByLabelScanClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_NODE_BY_LABEL_SCAN);
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;
	OpBase *clone = NewNodeByLabelScanOp(plan, op->n);
	return clone;
}

static void NodeByLabelScanFree(OpBase *op) {
	NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan *)op;

	GrB_Info info = RG_MatrixTupleIter_detach(&(nodeByLabelScan->iter));
	ASSERT(info == GrB_SUCCESS);

	if(nodeByLabelScan->child_record) {
		OpBase_DeleteRecord(nodeByLabelScan->child_record);
		nodeByLabelScan->child_record = NULL;
	}

	if(nodeByLabelScan->id_range) {
		UnsignedRange_Free(nodeByLabelScan->id_range);
		nodeByLabelScan->id_range = NULL;
	}
}

