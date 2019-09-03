/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"

/* Forward declarations */
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static Record Consume(OpBase *opBase);

OpBase *NewLimitOp(const ExecutionPlan *plan, unsigned int l) {
	OpLimit *op = malloc(sizeof(OpLimit));
	op->limit = l;
	op->consumed = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", NULL, Consume, Reset, NULL, Free, plan);
	return (OpBase *)op;
}

static Record Consume(OpBase *op) {
	OpLimit *limit = (OpLimit *)op;

	// Have we reached our limit?
	if(limit->consumed >= limit->limit) return NULL;

	// Consume a single record.
	limit->consumed++;
	OpBase *child = limit->op.children[0];
	return OpBase_Consume(child);
}

static OpResult Reset(OpBase *ctx) {
	OpLimit *limit = (OpLimit *)ctx;
	limit->consumed = 0;
	return OP_OK;
}

static void Free(OpBase *ctx) {
}
