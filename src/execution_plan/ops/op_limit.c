/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"

/* Forward declarations. */
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

OpBase *NewLimitOp(const ExecutionPlan *plan, unsigned int l) {
	OpLimit *op = malloc(sizeof(OpLimit));
	op->limit = l;
	op->consumed = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", NULL, Consume, Reset, NULL, Free, plan);

	return (OpBase *)op;
}

static Record Consume(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;

	// Have we reached our limit?
	if(op->consumed >= op->limit) return NULL;

	// Consume a single record.
	op->consumed++;
	OpBase *child = op->op.children[0];
	return OpBase_Consume(child);
}

static OpResult Reset(OpBase *ctx) {
	OpLimit *limit = (OpLimit *)ctx;
	limit->consumed = 0;
	return OP_OK;
}

static void Free(OpBase *ctx) {
}