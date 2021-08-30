/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "../../../RG.h"
#include "./ops/ops.h"
#include "runtime_execution_plan.h"

void _ExecutionPlan_Print(const RT_OpBase *op, RedisModuleCtx *ctx, sds *buffer,
						  int ident, int *op_count) {
	if(!op) return;

	*op_count += 1; // account for current operation.

	// Construct operation string representation.
	sdsclear(*buffer);
	*buffer = sdscatprintf(*buffer, "%*s", ident, "");
	RT_OpBase_ToString(op, buffer);

	RedisModule_ReplyWithStringBuffer(ctx, *buffer, sdslen(*buffer));

	// Recurse over child operations.
	for(int i = 0; i < op->childCount; i++) {
		_ExecutionPlan_Print(op->children[i], ctx, buffer, ident + 4, op_count);
	}
}

// Reply with a string representation of given execution plan.
void RT_ExecutionPlan_Print(const RT_ExecutionPlan *plan, RedisModuleCtx *ctx) {
	ASSERT(plan && ctx);

	int op_count = 0;   // Number of operations printed.
	sds buffer = sdsempty();

	// No idea how many operation are in execution plan.
	RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
	_ExecutionPlan_Print(plan->root, ctx, &buffer, 0, &op_count);

	RedisModule_ReplySetArrayLength(ctx, op_count);
	sdsfree(buffer);
}

