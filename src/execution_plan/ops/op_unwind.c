/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_unwind.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../../datatypes/array.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "limits.h"

#define INDEX_NOT_SET UINT_MAX

/* Forward declarations. */
static void UnwindFree(OpBase *opBase);

OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp) {
	OpUnwind *op = rm_malloc(sizeof(OpUnwind));

	op->exp = exp;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", NULL, UnwindFree, false,
		plan);

	OpBase_Modifies((OpBase *)op, exp->resolved_name);

	return (OpBase *)op;
}

static void UnwindFree(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;

	if(op->exp) {
		AR_EXP_Free(op->exp);
		op->exp = NULL;
	}
}
