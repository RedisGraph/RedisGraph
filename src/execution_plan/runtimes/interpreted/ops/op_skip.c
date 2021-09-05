/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"
#include "../../../../RG.h"
#include "../../../../errors.h"
#include "../../../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record SkipConsume(RT_OpBase *opBase);
static RT_OpResult SkipReset(RT_OpBase *opBase);
static void SkipFree(RT_OpBase *opBase);

static void _eval_skip(RT_OpSkip *op) {
	// Evaluate using the input expression, leaving the stored expression untouched.
	SIValue s = AR_EXP_Evaluate(op->skip_exp, NULL);

	// Validate that the skip value is numeric and non-negative.
	if(SI_TYPE(s) != T_INT64 || SI_GET_NUMERIC(s) < 0) {
		ErrorCtx_SetError("Skip operates only on non-negative integers");
	}

	op->skip = SI_GET_NUMERIC(s);
}

RT_OpBase *RT_NewSkipOp(const RT_ExecutionPlan *plan, const OpSkip *op_desc) {
	RT_OpSkip *op = rm_malloc(sizeof(RT_OpSkip));
	op->op_desc = op_desc;
	op->skip_exp = AR_EXP_Clone(op_desc->skip_exp);
	op->skipped = 0;

	_eval_skip(op);

	// set operations
	RT_OpBase_Init((RT_OpBase *)op, (const OpBase *)&op_desc->op, NULL, NULL,
		SkipConsume, SkipReset, SkipFree, plan);

	return (RT_OpBase *)op;
}

static Record SkipConsume(RT_OpBase *opBase) {
	RT_OpSkip *skip = (RT_OpSkip *)opBase;
	RT_OpBase *child = skip->op.children[0];

	// As long as we're required to skip
	while(skip->skipped < skip->skip) {
		Record discard = RT_OpBase_Consume(child);

		// Depleted.
		if(!discard) return NULL;

		// Discard.
		RT_OpBase_DeleteRecord(discard);

		// Advance.
		skip->skipped++;
	}

	return RT_OpBase_Consume(child);
}

static RT_OpResult SkipReset(RT_OpBase *ctx) {
	RT_OpSkip *skip = (RT_OpSkip *)ctx;
	skip->skipped = 0;
	return OP_OK;
}

static void SkipFree(RT_OpBase *opBase) {
	RT_OpSkip *op = (RT_OpSkip *)opBase;

	if(op->skip_exp != NULL) {
		AR_EXP_Free(op->skip_exp);
		op->skip_exp = NULL;
	}
}
