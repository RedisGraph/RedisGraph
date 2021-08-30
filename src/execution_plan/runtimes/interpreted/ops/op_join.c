/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_join.h"
#include "RG.h"
#include "../../../../query_ctx.h"

/* Forward declarations. */
static Record JoinConsume(RT_OpBase *opBase);
static RT_OpResult JoinReset(RT_OpBase *opBase);
static RT_OpResult JoinInit(RT_OpBase *opBase);

RT_OpBase *RT_NewJoinOp(const RT_ExecutionPlan *plan, const OpJoin *op_desc) {
	RT_OpJoin *op = rm_malloc(sizeof(RT_OpJoin));
	op->op_desc = op_desc;
	op->stream = NULL;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL,
		JoinInit, JoinConsume, NULL, NULL, plan);

	return (RT_OpBase *)op;
}

static RT_OpResult JoinInit(RT_OpBase *opBase) {
	RT_OpJoin *op = (RT_OpJoin *)opBase;
	// Start pulling from first stream.
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];
	return OP_OK;
}

static Record JoinConsume(RT_OpBase *opBase) {
	RT_OpJoin *op = (RT_OpJoin *)opBase;
	Record r = NULL;

	bool update_column_map = false;
	while(!r) {
		// Try pulling from current stream.
		r = RT_OpBase_Consume(op->stream);

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
			ResultSet_MapProjection(QueryCtx_GetResultSet(), r);
			update_column_map = false;
		}
	}

	return r;
}

static RT_OpResult JoinReset(RT_OpBase *opBase) {
	RT_OpJoin *op = (RT_OpJoin *)opBase;
	op->streamIdx = 0;
	op->stream = op->op.children[op->streamIdx];
	return OP_OK;
}
