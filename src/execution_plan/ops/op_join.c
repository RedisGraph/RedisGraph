/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_join.h"
#include "RG.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static Record JoinConsume(OpBase *opBase);
static OpResult JoinInit(OpBase *opBase);
static OpBase *JoinClone(const ExecutionPlan *plan, const OpBase *opBase);
static OpResult JoinReset(OpBase *opBase);

OpBase *NewJoinOp(const ExecutionPlan *plan) {
	OpJoin *op = rm_malloc(sizeof(OpJoin));
	op->stream = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_JOIN, "Join", JoinInit, JoinConsume, 
		JoinReset, NULL, JoinClone, NULL, false, plan);

	// default behavior is to update resultset mapping
	op->updateColumnMap = true;

	return (OpBase *)op;
}

static OpResult JoinInit(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	// Start pulling from first stream.
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];

	// map first stream resultset mapping
	ResultSet *result_set = QueryCtx_GetResultSet();
	// Note: Init should be called after `updateColumnMap` is externally set if
	// non-default behavior is desired
	if(result_set != NULL && op->updateColumnMap) {
		OpBase *child = op->stream;
		rax *mapping = ExecutionPlan_GetMappings(child->plan);
		ResultSet_MapProjection(result_set, mapping);
	}

	return OP_OK;
}

static Record JoinConsume(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	Record r = NULL;
	bool newStream = false;

	while(!r) {
		// Try pulling from current stream.
		r = OpBase_Consume(op->stream);

		if(!r) {
			// Stream depleted
			// Propagate reset to release RediSearch index lock if any exists
			OpBase_PropagateReset(op->stream);
			// See if there's a new stream to pull from.
			op->streamIdx++;
			if(op->streamIdx >= op->op.childCount) break;

			op->stream = op->op.children[op->streamIdx];
			// Switched streams, need to update the ResultSet column mapping
			newStream = true;
			continue;
		}

		if(op->updateColumnMap && newStream) {
			// We have a new record mapping, update the ResultSet column map to match it.
			ResultSet_MapProjection(QueryCtx_GetResultSet(), r->mapping);
		}
	}

	return r;
}

static inline OpBase *JoinClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_JOIN);
	OpBase *clone = NewJoinOp(plan);
	((OpJoin *)clone)->updateColumnMap = ((OpJoin *)opBase)->updateColumnMap;
	return clone;
}

static OpResult JoinReset
(
	OpBase *opBase
) {
	OpJoin *op = (OpJoin *)opBase;
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];

	// map first stream resultset mapping
	ResultSet *result_set = QueryCtx_GetResultSet();
	if(result_set != NULL && op->updateColumnMap) {
		OpBase *child = op->stream;
		rax *mapping = ExecutionPlan_GetMappings(child->plan);
		ResultSet_MapProjection(result_set, mapping);
	}

	return OP_OK;
}

void JoinSetUpdateColumnMap
(
	OpBase *op,
	bool update_column_map
) {
	ASSERT(op->type == OPType_JOIN);
	((OpJoin *)op)->updateColumnMap = update_column_map;
}

bool JoinGetUpdateColumnMap
(
	const OpBase *op
) {
	ASSERT(op->type == OPType_JOIN);
	return ((OpJoin *)op)->updateColumnMap;
}
