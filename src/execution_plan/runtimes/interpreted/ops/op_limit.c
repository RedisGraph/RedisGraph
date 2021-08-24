/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"
#include "../../../../RG.h"
#include "../../../../errors.h"
#include "../../../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record LimitConsume(RT_OpBase *opBase);
static RT_OpResult LimitReset(RT_OpBase *opBase);
static void LimitFree(RT_OpBase *opBase);
static RT_OpBase *LimitClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);

RT_OpBase *RT_NewLimitOp(const RT_ExecutionPlan *plan, uint limit, AR_ExpNode *limit_exp) {
	// validate inputs
	ASSERT(plan != NULL);
	ASSERT(limit_exp != NULL);

	RT_OpLimit *op = rm_malloc(sizeof(RT_OpLimit));
	op->limit = limit;
	op->consumed = 0;
	op->limit_exp = limit_exp;

	// set operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_LIMIT, NULL, LimitConsume, LimitReset,
				LimitClone, LimitFree, false, plan);

	return (RT_OpBase *)op;
}

static Record LimitConsume(RT_OpBase *opBase) {
	RT_OpLimit *op = (RT_OpLimit *)opBase;

	// Have we reached our limit?
	if(op->consumed >= op->limit) return NULL;

	// Consume a single record.
	op->consumed++;
	RT_OpBase *child = op->op.children[0];
	return RT_OpBase_Consume(child);
}

static RT_OpResult LimitReset(RT_OpBase *ctx) {
	RT_OpLimit *limit = (RT_OpLimit *)ctx;
	limit->consumed = 0;
	return OP_OK;
}

static inline RT_OpBase *LimitClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_LIMIT);

	RT_OpLimit *op = (RT_OpLimit *)opBase;
	/* Clone the limit expression stored on the ExecutionPlan,
	 * as we don't want to modify the templated ExecutionPlan
	 * (which may occur if this expression is a parameter). */
	AR_ExpNode *limit_exp = AR_EXP_Clone(op->limit_exp);
	return RT_NewLimitOp(plan, op->limit, limit_exp);
}

static void LimitFree(RT_OpBase *opBase) {
	RT_OpLimit *op = (RT_OpLimit *)opBase;

	if(op->limit_exp) {
		AR_EXP_Free(op->limit_exp);
		op->limit_exp = NULL;
	}
}
