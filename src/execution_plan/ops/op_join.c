/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_join.h"
#include "RG.h"
#include "../../query_ctx.h"

OpBase *NewJoinOp(const ExecutionPlan *plan) {
	OpJoin *op = rm_malloc(sizeof(OpJoin));

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_JOIN, "Join", NULL, false, plan);

	return (OpBase *)op;
}
