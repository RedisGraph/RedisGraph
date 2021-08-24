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
static RT_OpBase *SkipClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);

RT_OpBase *RT_NewSkipOp(const RT_ExecutionPlan *plan, uint skip, AR_ExpNode *skip_exp) {
	RT_OpSkip *op = rm_malloc(sizeof(RT_OpSkip));
	op->skip = skip;
	op->skipped = 0;
	op->skip_exp = skip_exp;

	// set operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_SKIP, NULL, SkipConsume, SkipReset, SkipClone,
				SkipFree, false, plan);

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

static inline RT_OpBase *SkipClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_SKIP);

	RT_OpSkip *op = (RT_OpSkip *)opBase;
	/* Clone the skip expression stored on the ExecutionPlan,
	 * as we don't want to modify the templated ExecutionPlan
	 * (which may occur if this expression is a parameter). */
	AR_ExpNode *skip_exp = AR_EXP_Clone(op->skip_exp);
	return RT_NewSkipOp(plan, op->skip, skip_exp);
}

static void SkipFree(RT_OpBase *opBase) {
	RT_OpSkip *op = (RT_OpSkip *)opBase;

	if(op->skip_exp != NULL) {
		AR_EXP_Free(op->skip_exp);
		op->skip_exp = NULL;
	}
}
