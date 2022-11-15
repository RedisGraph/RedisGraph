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

OpBase *NewJoinOp(const ExecutionPlan *plan) {
	OpJoin *op = rm_malloc(sizeof(OpJoin));
	op->stream = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_JOIN, "Join", JoinInit, JoinConsume, 
		NULL, NULL, JoinClone, NULL, false, plan);

	return (OpBase *)op;
}

static OpResult JoinInit(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	// Start pulling from first stream.
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];

	// map first stream resultset mapping
	ResultSet *result_set = QueryCtx_GetResultSet();
	if(result_set != NULL) {
		OpBase *child = op->stream;
		rax *mapping = ExecutionPlan_GetMappings(child->plan);
		ResultSet_MapProjection(result_set, mapping);
	}

	return OP_OK;
}

static Record JoinConsume(OpBase *opBase) {
	OpJoin *op = (OpJoin *)opBase;
	Record r = NULL;
	bool update_column_map = false;

	while(!r) {
		// Try pulling from current stream.
		r = OpBase_Consume(op->stream);

		if(!r) {
			// Stream depleted, see if there's a new stream to pull from.
			op->streamIdx++;
			if(op->streamIdx >= op->op.childCount) break;

			op->stream = op->op.children[op->streamIdx];
			// Switched streams, need to update the ResultSet column mapping
			update_column_map = true;
			continue;
		}

		if(update_column_map) {
			// We have a new record mapping, update the ResultSet column map to match it.
			ResultSet_MapProjection(QueryCtx_GetResultSet(), r->mapping);
		}
	}

	return r;
}

static inline OpBase *JoinClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_JOIN);
	return NewJoinOp(plan);
}

