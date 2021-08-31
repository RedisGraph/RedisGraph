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

static void _eval_limit(RT_OpLimit *op) {
	// Evaluate using the input expression, leaving the stored expression untouched.
	SIValue l = AR_EXP_Evaluate(op->op_desc->limit_exp, NULL);

	// Validate that the limit value is numeric and non-negative.
	if(SI_TYPE(l) != T_INT64 || SI_GET_NUMERIC(l) < 0) {
		ErrorCtx_SetError("Limit operates only on non-negative integers");
	}

	op->limit = SI_GET_NUMERIC(l);
}

RT_OpBase *RT_NewLimitOp(const RT_ExecutionPlan *plan, const OpLimit *op_desc) {
	// validate inputs
	ASSERT(plan != NULL);
	ASSERT(op_desc != NULL);

	RT_OpLimit *op = rm_malloc(sizeof(RT_OpLimit));
	op->op_desc = op_desc;
	op->consumed = 0;

	_eval_limit(op);

	// set operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL, NULL,
		LimitConsume, LimitReset, NULL, plan);

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
