/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_join.h"

/* Forward declarations. */
static Record JoinConsume(OpBase *opBase);
static OpResult JoinReset(OpBase *opBase);
static OpResult JoinInit(OpBase *opBase);

OpBase *NewJoinOp(const ExecutionPlan *plan) {
	OpJoin *op = rm_malloc(sizeof(OpJoin));
	op->stream = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_JOIN, "Join", JoinInit, JoinConsume, NULL, NULL, NULL, plan);

	return (OpBase *)op;
}

static OpResult JoinInit(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	// Start pulling from first stream.
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];
	return OP_OK;
}

static Record JoinConsume(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	Record r = NULL;

	while(true) {
		// Try pulling from current stream.
		r = OpBase_Consume(op->stream);
		// Managed to get a record, break out.
		if(r) break;

		// Stream depleted, see if there's a new stream to pull from.
		if(op->streamIdx < op->op.childCount) {
			op->streamIdx++;
			op->stream = op->op.children[op->streamIdx];
			// Re-enter loop.
			continue;
		}
		break;
	}

	return r;
}

static OpResult JoinReset(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];
	return OP_OK;
}
