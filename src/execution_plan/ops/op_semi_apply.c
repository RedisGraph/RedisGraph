/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_semi_apply.h"
#include "../execution_plan.h"
#include "../execution_plan_build/execution_plan_modify.h"

OpBase *NewSemiApplyOp(const ExecutionPlan *plan, bool anti) {
	OpSemiApply *op = rm_malloc(sizeof(OpSemiApply));
	op->op_arg = NULL;
	op->bound_branch = NULL;
	op->match_branch = NULL;
	// Set our Op operations
	if(anti) {
		OpBase_Init((OpBase *)op, OPType_ANTI_SEMI_APPLY, "Anti Semi Apply",
			NULL, false, plan);
	} else {
		OpBase_Init((OpBase *)op, OPType_SEMI_APPLY, "Semi Apply",
			NULL, false, plan);
	}
	return (OpBase *) op;
}
