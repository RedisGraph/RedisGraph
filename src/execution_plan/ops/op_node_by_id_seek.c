/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_id_seek.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult NodeByIdSeekInit(OpBase *opBase);
static Record NodeByIdSeekConsume(OpBase *opBase);
static Record NodeByIdSeekConsumeFromChild(OpBase *opBase);
static OpResult NodeByIdSeekReset(OpBase *opBase);
static void NodeByIdSeekFree(OpBase *opBase);

// Checks to see if operation index is within its bounds.
static inline bool _outOfBounds(NodeByIdSeek *op) {
	/* Because currentId starts at minimum and only increases
	 * we only care about top bound. */
	if(op->currentId > op->maxId) return true;
	if(op->currentId == op->maxId && !op->maxInclusive) return true;
	return false;
}

OpBase *NewNodeByIdSeekOp
(
	const ExecutionPlan *plan,
	const QGNode *node,
	NodeID minId,
	NodeID maxId,
	bool minInclusive,
	bool maxInclusive
) {
	// Can't include unspecified bound.
	assert(!(minId == ID_RANGE_UNBOUND && minInclusive));
	assert(!(maxId == ID_RANGE_UNBOUND && maxInclusive));

	NodeByIdSeek *op = malloc(sizeof(NodeByIdSeek));
	op->g = QueryCtx_GetGraph();
	op->child_record = NULL;

	op->minInclusive = minInclusive;
	op->maxInclusive = maxInclusive;

	// The smallest possible entity ID is 0.
	op->minId = minId;
	if(minId == ID_RANGE_UNBOUND) op->minId = 0;

	// The largest possible entity ID is the same as Graph_RequiredMatrixDim.
	if(maxId == ID_RANGE_UNBOUND) maxId = Graph_RequiredMatrixDim(op->g);
	op->maxId = MIN(Graph_RequiredMatrixDim(op->g), maxId);

	op->currentId = op->minId;
	/* Advance current ID when min is not inclusive and
	 * minimum range is specified. */
	if(!minInclusive && minId != ID_RANGE_UNBOUND) op->currentId++;


	OpBase_Init((OpBase *)op, OPType_NODE_BY_ID_SEEK, "NodeByIdSeek", NodeByIdSeekInit,
				NodeByIdSeekConsume, NodeByIdSeekReset, NULL, NodeByIdSeekFree, plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, node->alias);

	return (OpBase *)op;
}

static OpResult NodeByIdSeekInit(OpBase *opBase) {
	if(opBase->childCount > 0) opBase->consume = NodeByIdSeekConsumeFromChild;
	return OP_OK;
}

static inline Node _SeekNextNode(NodeByIdSeek *op) {
	Node n = { .entity = NULL };

	/* As long as we're within range bounds
	 * and we've yet to get a node. */
	while(!_outOfBounds(op)) {
		if(Graph_GetNode(op->g, op->currentId, &n)) break;
		op->currentId++;
	}

	// Advance id for next consume call.
	op->currentId++;

	// Did we manage to get an entity?
	if(!n.entity) return n;
	// Null-set the label in case an operation (like op_delete) accesses it.
	// TODO If we're replacing a label scan, the correct label can be populated now.
	n.label = NULL;

	return n;
}

static Record NodeByIdSeekConsumeFromChild(OpBase *opBase) {
	NodeByIdSeek *op = (NodeByIdSeek *)opBase;

	if(op->child_record == NULL) {
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL;
		else NodeByIdSeekReset(opBase);
	}

	Node n = _SeekNextNode(op);

	if(n.entity == NULL) { // Failed to retrieve a node.
		Record_Free(op->child_record); // Free old record.
		// Pull a new record from child.
		op->child_record = OpBase_Consume(op->op.children[0]);
		if(op->child_record == NULL) return NULL; // Child depleted.

		// Reset iterator and evaluate again.
		NodeByIdSeekReset(opBase);
		n = _SeekNextNode(op);
		if(n.entity == NULL) return NULL; // Empty iterator; return immediately.
	}

	// Clone the held Record, as it will be freed upstream.
	Record r = Record_Clone(op->child_record);

	// Populate the Record with the actual node.
	Record_AddNode(r, op->nodeRecIdx, n);

	return r;
}

static Record NodeByIdSeekConsume(OpBase *opBase) {
	NodeByIdSeek *op = (NodeByIdSeek *)opBase;

	Node n = _SeekNextNode(op);
	if(n.entity == NULL) return NULL; // Failed to retrieve a node.

	// Create a new Record.
	Record r = OpBase_CreateRecord(opBase);

	// Populate the Record with the actual node.
	Record_AddNode(r, op->nodeRecIdx, n);

	return r;
}

static OpResult NodeByIdSeekReset(OpBase *ctx) {
	NodeByIdSeek *op = (NodeByIdSeek *)ctx;
	op->currentId = op->minId;
	if(!op->minInclusive) op->currentId++;
	return OP_OK;
}

static void NodeByIdSeekFree(OpBase *opBase) {
	NodeByIdSeek *op = (NodeByIdSeek *)opBase;
	if(op->child_record) {
		Record_Free(op->child_record);
		op->child_record = NULL;
	}
}

