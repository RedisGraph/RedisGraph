/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"
#include "../../RG.h"
#include "../../errors.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record SkipConsume(OpBase *opBase);
static OpResult SkipReset(OpBase *opBase);
static void SkipFree(OpBase *opBase);
static OpBase *SkipClone(const ExecutionPlan *plan, const OpBase *opBase);

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
	op->skipped = 0;
	op->skip_exp = NULL;

	_eval_skip(op, skip_exp);

	// set operations
	OpBase_Init((OpBase *)op, OPType_SKIP, "Skip", NULL, SkipConsume, SkipReset, NULL, SkipClone,
				SkipFree, false, plan);

	return (OpBase *)op;
}

static void _skip(OpBase *opBase) {
	OpSkip *op = (OpSkip *)opBase;
	RecordBatch batch = NULL;
	OpBase *child = op->op.children[0];

	// determine how many batchs are going to be skipped
	uint batches_to_skip = op->skip / EXEC_PLAN_BATCH_SIZE;

	// skip entire batches
	for(uint i = 0; i < batches_to_skip; i++) {
		batch = OpBase_Consume(child);
		if(RecordBatch_Empty(batch)) return;

		// discard records
		uint batch_size = RecordBatch_Len(batch);
		for(uint i = 0; i < batch_size; i++) OpBase_DeleteRecord(batch[i]);
	}

	op->skipped = batches_to_skip * EXEC_PLAN_BATCH_SIZE;

	// skip half batch
	if(op->skipped < op->skip) {
		batch = OpBase_Consume(child);
		if(RecordBatch_Empty(batch)) return;

		uint i = 0;
		for(; op->skipped < op->skip; op->skipped++, i++) {
			OpBase_DeleteRecord(batch[i]);
		}

		ASSERT(op->skipped == op->skip);

		// add remaining records
		uint remaining_record_count = RecordBatch_Len(batch);
		for(; i < remaining_record_count; i++) OP_BATCH_ADD(batch[i]);
	}
}

static Record SkipConsume(OpBase *opBase) {
	OpSkip *op = (OpSkip *)opBase;
	OpBase *child = op->op.children[0];

	if(op->skipped < op->skip) {
		OP_BATCH_CLEAR();
		_skip(opBase);
		if(!OP_BATCH_EMPTY()) OP_BATCH_EMIT();
	}

	// done skipping, return child batch
	return OpBase_Consume(child);
}

static OpResult SkipReset(OpBase *ctx) {
	OpSkip *skip = (OpSkip *)ctx;
	skip->skipped = 0;
	return OP_OK;
}

static inline OpBase *SkipClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_SKIP);

	OpSkip *op = (OpSkip *)opBase;
	/* Clone the skip expression stored on the ExecutionPlan,
	 * as we don't want to modify the templated ExecutionPlan
	 * (which may occur if this expression is a parameter). */
	AR_ExpNode *skip_exp = AR_EXP_Clone(op->skip_exp);
	return NewSkipOp(plan, skip_exp);
}

static void SkipFree(OpBase *opBase) {
	OpSkip *op = (OpSkip *)opBase;

	if(op->skip_exp != NULL) {
		AR_EXP_Free(op->skip_exp);
		op->skip_exp = NULL;
	}
}

