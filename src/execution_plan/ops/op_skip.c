/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult SkipInit(OpBase *opBase);
static Record SkipConsume(OpBase *opBase);
static OpResult SkipReset(OpBase *opBase);
static OpBase *SkipClone(const ExecutionPlan *plan, const OpBase *opBase);
static void SkipFree(OpBase *opBase);

OpBase *NewSkipOp(const ExecutionPlan *plan, AR_ExpNode *skip_expr) {
	OpSkip *op = rm_malloc(sizeof(OpSkip));
	op->skipped = 0;
	op->skip_expr = skip_expr;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SKIP, "Skip", SkipInit, SkipConsume, SkipReset, NULL, SkipClone,
				NULL, false, plan);

	return (OpBase *)op;
}

static OpResult SkipInit(OpBase *opBase) {
	OpSkip *skip = (OpSkip *)opBase;
	skip->rec_to_skip = 0;
	if(skip->skip_expr) {
		SIValue skip_value =  AR_EXP_Evaluate(skip->skip_expr, NULL);
		if(SI_TYPE(skip_value) != T_INT64) {
			char *error;
			asprintf(&error, "SKIP specified value of invalid type, must be a positive integer");
			QueryCtx_SetError(error); // Set the query-level error.
			QueryCtx_RaiseRuntimeException();
			skip->rec_to_skip = 0;
			return OP_ERR;
		}
		skip->rec_to_skip = skip_value.longval;
	}
	return OP_OK;
}

static Record SkipConsume(OpBase *opBase) {
	OpSkip *skip = (OpSkip *)opBase;
	OpBase *child = skip->op.children[0];

	// As long as we're required to skip
	while(skip->skipped < skip->rec_to_skip) {
		Record discard = OpBase_Consume(child);

		// Depleted.
		if(!discard) return NULL;

		// Discard.
		OpBase_DeleteRecord(discard);

		// Advance.
		skip->skipped++;
	}

	return OpBase_Consume(child);
}

static OpResult SkipReset(OpBase *ctx) {
	OpSkip *skip = (OpSkip *)ctx;
	skip->skipped = 0;
	return OP_OK;
}

static OpBase *SkipClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_SKIP);
	OpSkip *op = (OpSkip *)opBase;
	return NewSkipOp(plan, AR_EXP_Clone(op->skip_expr));
}

static inline void SkipFree(OpBase *opBase) {
	OpSkip *op = (OpSkip *)opBase;
	if(op->skip_expr) {
		AR_EXP_Free(op->skip_expr);
		op->skip_expr = NULL;
	}
}
