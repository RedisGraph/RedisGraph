/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static void SkipFree(OpBase *opBase);

static void _eval_skip(OpSkip *op, AR_ExpNode *skip_exp) {
	/* Store a copy of the original expression.
	 * This is required in the case of a parameterized skip: "SKIP $L"
	 * Evaluating the expression will modify it, replacing the parameter with a constant.
	 * As a result, clones of this operation would invalidly resolve to an outdated constant. */
	op->skip_exp = AR_EXP_Clone(skip_exp);

	// Evaluate using the input expression, leaving the stored expression untouched.
	SIValue s = AR_EXP_Evaluate(skip_exp, NULL);

	// Validate that the skip value is numeric and non-negative.
	if(SI_TYPE(s) != T_INT64 || SI_GET_NUMERIC(s) < 0) {
		ErrorCtx_SetError("Skip operates only on non-negative integers");
	}

	op->skip = SI_GET_NUMERIC(s);

	// Free the expression we've evaluated.
	AR_EXP_Free(skip_exp);
}

OpBase *NewSkipOp(const ExecutionPlan *plan, AR_ExpNode *skip_exp) {
	OpSkip *op = rm_malloc(sizeof(OpSkip));
	op->skip = 0;
	op->skip_exp = NULL;

	_eval_skip(op, skip_exp);

	// set operations
	OpBase_Init((OpBase *)op, OPType_SKIP, "Skip", SkipFree, false, plan);

	return (OpBase *)op;
}

static void SkipFree(OpBase *opBase) {
	OpSkip *op = (OpSkip *)opBase;

	if(op->skip_exp != NULL) {
		AR_EXP_Free(op->skip_exp);
		op->skip_exp = NULL;
	}
}
