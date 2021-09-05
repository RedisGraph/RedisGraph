/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static void LimitFree(OpBase *opBase);

OpBase *NewLimitOp(const ExecutionPlan *plan, AR_ExpNode *limit_exp) {
	// validate inputs
	ASSERT(plan != NULL);
	ASSERT(limit_exp != NULL);

	OpLimit *op = rm_malloc(sizeof(OpLimit));
	op->limit_exp = limit_exp;

	// set operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", LimitFree, false, plan);

	return (OpBase *)op;
}

static void LimitFree(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;

	if(op->limit_exp) {
		AR_EXP_Free(op->limit_exp);
		op->limit_exp = NULL;
	}
}
