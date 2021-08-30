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
	op->limit_exp = NULL;

	_eval_limit(op, limit_exp);

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
