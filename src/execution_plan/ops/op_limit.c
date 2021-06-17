/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record LimitConsume(OpBase *opBase);
static OpResult LimitReset(OpBase *opBase);
static void LimitFree(OpBase *opBase);
static OpBase *LimitClone(const ExecutionPlan *plan, const OpBase *opBase);

static void _eval_limit(OpLimit *op, AR_ExpNode *limit_exp) {
	/* Store a copy of the original expression.
	 * This is required in the case of a parameterized limit: "LIMIT $L"
	 * Evaluating the expression will modify it, replacing the parameter with a constant.
	 * As a result, clones of this operation would invalidly resolve to an outdated constant. */
	op->limit_exp = AR_EXP_Clone(limit_exp);

	// Evaluate using the input expression, leaving the stored expression untouched.
	SIValue l = AR_EXP_Evaluate(limit_exp, NULL);

	// Validate that the limit value is numeric and non-negative.
	if(SI_TYPE(l) != T_INT64 || SI_GET_NUMERIC(l) < 0) {
		ErrorCtx_SetError("Limit operates only on non-negative integers");
	}

	op->limit = SI_GET_NUMERIC(l);

	// Free the expression we've evaluated.
	AR_EXP_Free(limit_exp);
}

OpBase *NewLimitOp(const ExecutionPlan *plan, AR_ExpNode *limit_exp) {
	// validate inputs
	ASSERT(plan != NULL);
	ASSERT(limit_exp != NULL);

	OpLimit *op = rm_malloc(sizeof(OpLimit));
	op->limit = 0;
	op->consumed = 0;
	op->limit_exp = NULL;

	_eval_limit(op, limit_exp);

	// set operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", NULL, LimitConsume, LimitReset, NULL,
				LimitClone, LimitFree, false, plan);

	return (OpBase *)op;
}

static Record LimitConsume(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;

	// Have we reached our limit?
	if(op->consumed >= op->limit) return NULL;

	// Consume a single record.
	op->consumed++;
	OpBase *child = op->op.children[0];
	return OpBase_Consume(child);
}

static OpResult LimitReset(OpBase *ctx) {
	OpLimit *limit = (OpLimit *)ctx;
	limit->consumed = 0;
	return OP_OK;
}

static inline OpBase *LimitClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_LIMIT);

	OpLimit *op = (OpLimit *)opBase;
	/* Clone the limit expression stored on the ExecutionPlan,
	 * as we don't want to modify the templated ExecutionPlan
	 * (which may occur if this expression is a parameter). */
	AR_ExpNode *limit_exp = AR_EXP_Clone(op->limit_exp);
	return NewLimitOp(plan, limit_exp);
}

static void LimitFree(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;

	if(op->limit_exp) {
		AR_EXP_Free(op->limit_exp);
		op->limit_exp = NULL;
	}
}

