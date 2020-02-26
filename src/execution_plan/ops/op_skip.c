/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"

/* Forward declarations. */
static Record SkipConsume(OpBase *opBase);
static OpResult SkipReset(OpBase *opBase);
static OpBase *SkipClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewSkipOp(const ExecutionPlan *plan, unsigned int rec_to_skip) {
	OpSkip *op = rm_malloc(sizeof(OpSkip));
	op->skipped = 0;
	op->rec_to_skip = rec_to_skip;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SKIP, "Skip", NULL, SkipConsume, SkipReset, NULL, SkipClone, NULL,
				false, plan);

	return (OpBase *)op;
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
	return NewSkipOp(plan, op->rec_to_skip);
}
