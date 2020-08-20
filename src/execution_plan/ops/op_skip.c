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

OpBase *NewSkipOp(const ExecutionPlan *plan) {
	OpSkip *op = rm_malloc(sizeof(OpSkip));
	op->skipped = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_SKIP, "Skip", SkipInit, SkipConsume, SkipReset, NULL, SkipClone,
				NULL, false, plan);

	return (OpBase *)op;
}

static OpResult SkipInit(OpBase *opBase) {
	OpSkip *skip = (OpSkip *)opBase;
	AST *ast = ExecutionPlan_GetAST(opBase->plan);
	skip->rec_to_skip = AST_GetSkip(ast);
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

static inline OpBase *SkipClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_SKIP);
	return NewSkipOp(plan);
}
