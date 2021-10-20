/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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

OpBase *NewNodeByLabelScanOp(const ExecutionPlan *plan, NodeScanCtx n) {
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

static GrB_Info _ConstructIterator(NodeByLabelScan *op, Schema *schema) {
	NodeID minId;
	NodeID maxId;
	GrB_Info info;
	GrB_Index nrows;

	GraphContext  *gc  =  QueryCtx_GetGraphCtx();
	RG_Matrix     L    =  Graph_GetLabelMatrix(gc->g, schema->id);

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

	info = RG_MatrixTupleIter_new(&(op->iter), L);
	ASSERT(info == GrB_SUCCESS);

	// use range only when minId and maxId are subset of the entire matrix
	if(minId > 0 || maxId < nrows-1) {
		info = RG_MatrixTupleIter_iterate_range(op->iter, minId, maxId);
	}

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

	// If we have no children, we can build the iterator now.
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Schema *schema = GraphContext_GetSchema(gc, op->n.label, SCHEMA_NODE);
	if(!schema) {
		// Missing schema, use the NOP consume function.
		OpBase_UpdateConsume(opBase, NodeByLabelScanNoOp);
		return OP_OK;
	}
	// Resolve label ID at runtime.
	op->n.label_id = schema->id;

	// The iterator build may fail if the ID range does not match the matrix dimensions.
	GrB_Info iterator_built = _ConstructIterator(op, schema);
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
	NodeID minId = op->id_range->include_min ? op->id_range->min : op->id_range->min + 1;
	NodeID maxId = op->id_range->include_max ? op->id_range->max : op->id_range->max - 1 ;
	RG_MatrixTupleIter_iterate_range(op->iter, minId, maxId);
}

static Record NodeByLabelScanConsumeFromChild(OpBase *opBase) {
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;

	// Try to get new nodeID.
	GrB_Index nodeId;
	bool depleted = true;
	RG_MatrixTupleIter_next(op->iter, NULL, &nodeId, NULL, &depleted);
	/* depleted will be true in the following cases:
	 * 1. No iterator: GxB_MatrixTupleIter_next will fail and depleted will stay true. This scenario means
	 * that there was no consumption of a record from a child, otherwise there was an iterator.
	 * 2. Iterator depleted - For every child record the iterator finished the entire matrix scan and it needs to restart.
	 * The child record will be NULL if this is the op's first invocation or it has just been reset, in which case we
	 * should also enter this loop. */
	while(depleted || op->child_record == NULL) {
		// Try to get a record.
		if(op->child_record) OpBase_DeleteRecord(op->child_record);
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;

		// Got a record.
		if(!op->iter) {
			// Iterator wasn't set up until now.
			GraphContext *gc = QueryCtx_GetGraphCtx();
			Schema *schema = GraphContext_GetSchema(gc, op->n.label, SCHEMA_NODE);
			// No label matrix, it might be created in the next iteration.
			if(!schema) continue;
			if(_ConstructIterator(op, schema) != GrB_SUCCESS) continue;
		} else {
			// Iterator depleted - reset.
			// TODO: GxB_MatrixTupleIter_reset
			_ResetIterator(op);
		}
		// Try to get new NodeID.
		RG_MatrixTupleIter_next(op->iter, NULL, &nodeId, NULL, &depleted);
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
	RG_MatrixTupleIter_next(op->iter, NULL, &nodeId, NULL, &depleted);
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

static OpBase *NodeByLabelScanClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_NODE_BY_LABEL_SCAN);
	NodeByLabelScan *op = (NodeByLabelScan *)opBase;
	OpBase *clone = NewNodeByLabelScanOp(plan, op->n);
	return clone;
}

static void NodeByLabelScanFree(OpBase *op) {
	NodeByLabelScan *nodeByLabelScan = (NodeByLabelScan *)op;

	if(nodeByLabelScan->iter) {
		RG_MatrixTupleIter_free(&(nodeByLabelScan->iter));
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

