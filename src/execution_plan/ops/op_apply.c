/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply.h"

OpBase *NewApplyOp(const ExecutionPlan *plan) {
	Apply *op = rm_malloc(sizeof(Apply));
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->rhs_branch = NULL;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_APPLY, "Apply", NULL, false, plan);

	return (OpBase *)op;
}
