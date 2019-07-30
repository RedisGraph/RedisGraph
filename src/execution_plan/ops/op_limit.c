/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"

OpBase *NewLimitOp(unsigned int l) {
	OpLimit *limit = malloc(sizeof(OpLimit));
	limit->limit = l;
	limit->consumed = 0;

	// Set our Op operations
	OpBase_Init(&limit->op);
	limit->op.name = "Limit";
	limit->op.type = OPType_LIMIT;
	limit->op.consume = LimitConsume;
	limit->op.reset = LimitReset;
	limit->op.free = LimitFree;

	return (OpBase *)limit;
}

Record LimitConsume(OpBase *op) {
	OpLimit *limit = (OpLimit *)op;

	// Have we reached our limit?
	if(limit->consumed >= limit->limit) return NULL;

	// Consume a single record.
	limit->consumed++;
	OpBase *child = limit->op.children[0];
	return OpBase_Consume(child);
}

OpResult LimitReset(OpBase *ctx) {
	OpLimit *limit = (OpLimit *)ctx;
	limit->consumed = 0;
	return OP_OK;
}

void LimitFree(OpBase *ctx) {
}