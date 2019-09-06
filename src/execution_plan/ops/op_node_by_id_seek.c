/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_node_by_id_seek.h"
#include "../../query_ctx.h"

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
	unsigned int nodeRecIdx,
	NodeID minId,
	NodeID maxId,
	bool minInclusive,
	bool maxInclusive
) {
	// Can't include unspecified bound.
	assert(!(minId == ID_RANGE_UNBOUND && minInclusive));
	assert(!(maxId == ID_RANGE_UNBOUND && maxInclusive));

	OpNodeByIdSeek *op_nodeByIdSeek = malloc(sizeof(OpNodeByIdSeek));
	op_nodeByIdSeek->g = QueryCtx_GetGraph();

	op_nodeByIdSeek->minInclusive = minInclusive;
	op_nodeByIdSeek->maxInclusive = maxInclusive;

	// The smallest possible entity ID is 0.
	op_nodeByIdSeek->minId = minId;
	if(minId == ID_RANGE_UNBOUND) op_nodeByIdSeek->minId = 0;

	// The largest possible entity ID is the same as Graph_RequiredMatrixDim.
	if(maxId == ID_RANGE_UNBOUND) maxId = Graph_RequiredMatrixDim(op_nodeByIdSeek->g);
	op_nodeByIdSeek->maxId = MIN(Graph_RequiredMatrixDim(op_nodeByIdSeek->g), maxId);

	op_nodeByIdSeek->currentId = op_nodeByIdSeek->minId;
	/* Advance current ID when min is not inclusive and
	 * minimum range is specified. */
	if(!minInclusive && minId != ID_RANGE_UNBOUND) op_nodeByIdSeek->currentId++;

	op_nodeByIdSeek->nodeRecIdx = nodeRecIdx;

	OpBase_Init(&op_nodeByIdSeek->op);
	op_nodeByIdSeek->op.name = "NodeByIdSeek";
	op_nodeByIdSeek->op.type = OPType_NODE_BY_ID_SEEK;
	op_nodeByIdSeek->op.consume = OpNodeByIdSeekConsume;
	op_nodeByIdSeek->op.init = OpNodeByIdSeekInit;
	op_nodeByIdSeek->op.reset = OpNodeByIdSeekReset;
	op_nodeByIdSeek->op.free = OpNodeByIdSeekFree;

	return (OpBase *)op_nodeByIdSeek;
}

OpResult OpNodeByIdSeekInit(OpBase *opBase) {
	return OP_OK;
}

Record OpNodeByIdSeekConsume(OpBase *opBase) {
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

	Record r = Record_New(opBase->record_map->record_len);
	Record_AddNode(r, op->nodeRecIdx, n);
	return r;
}

OpResult OpNodeByIdSeekReset(OpBase *ctx) {
	OpNodeByIdSeek *op = (OpNodeByIdSeek *)ctx;
	op->currentId = op->minId;
	if(!op->minInclusive) op->currentId++;
	return OP_OK;
}

void OpNodeByIdSeekFree(OpBase *ctx) {

}

