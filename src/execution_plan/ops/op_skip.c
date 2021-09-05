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

OpBase *NewSkipOp(const ExecutionPlan *plan, AR_ExpNode *skip_exp) {
	OpSkip *op = rm_malloc(sizeof(OpSkip));
	op->skip_exp = skip_exp;

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
