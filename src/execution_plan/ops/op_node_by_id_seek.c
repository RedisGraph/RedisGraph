/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_id_seek.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

// Checks to see if operation index is within its bounds.
static inline bool _outOfBounds(OpNodeByIdSeek *op) {
	/* Because currentId starts at minimum and only increases
	 * we only care about top bound. */
	if(op->currentId > op->maxId) return true;
	if(op->currentId == op->maxId && !op->maxInclusive) return true;
	return false;
}

OpBase *NewOpNodeByIdSeekOp
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

	OpNodeByIdSeek *op = malloc(sizeof(OpNodeByIdSeek));
	op->g = QueryCtx_GetGraph();

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


	OpBase_Init((OpBase *)op, OPType_NODE_BY_ID_SEEK, "NodeByIdSeek", Init, Consume, Reset, NULL, Free,
				plan);

	op->nodeRecIdx = OpBase_Modifies((OpBase *)op, node->alias);

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	OpNodeByIdSeek *op = (OpNodeByIdSeek *)opBase;
	Node n;
	n.entity = NULL;

	/* As long as we're within range bounds
	 * and we've yet to get a node. */
	while(!_outOfBounds(op)) {
		if(Graph_GetNode(op->g, op->currentId, &n)) break;
		op->currentId++;
	}

	// Advance id for next consume call.
	op->currentId++;

	// Did we managed to get an entity?
	if(!n.entity) return NULL;
	// Null-set the label in case an operation (like op_delete) accesses it.
	// TODO If we're replacing a label scan, the correct label can be populated now.
	n.label = NULL;

	Record r = OpBase_CreateRecord((OpBase *)op);
	Record_AddNode(r, op->nodeRecIdx, n);
	return r;
}

static OpResult Reset(OpBase *ctx) {
	OpNodeByIdSeek *op = (OpNodeByIdSeek *)ctx;
	op->currentId = op->minId;
	if(!op->minInclusive) op->currentId++;
	return OP_OK;
}

static void Free(OpBase *ctx) {

}

