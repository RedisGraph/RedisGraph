/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"

/* Forward declarations. */
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

OpBase *NewSkipOp(const ExecutionPlan *plan, unsigned int rec_to_skip) {
	OpSkip *op = malloc(sizeof(OpSkip));
	op->skipped = 0;
	op->rec_to_skip = rec_to_skip;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SKIP, "Skip", NULL, Consume, Reset, NULL, Free, plan);

	return (OpBase *)op;
}

static Record Consume(OpBase *opBase) {
	OpSkip *skip = (OpSkip *)opBase;
	OpBase *child = skip->op.children[0];

	// As long as we're required to skip
	while(skip->skipped < skip->rec_to_skip) {
		Record discard = OpBase_Consume(child);

		// Depleted.
		if(!discard) return NULL;

		// Discard.
		Record_Free(discard);

		// Advance.
		skip->skipped++;
	}

	return OpBase_Consume(child);
}

static OpResult Reset(OpBase *ctx) {
	OpSkip *skip = (OpSkip *)ctx;
	skip->skipped = 0;
	return OP_OK;
}

static void Free(OpBase *ctx) {

}
