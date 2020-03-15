/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static OpResult LimitInit(OpBase *opBase);
static Record LimitConsume(OpBase *opBase);
static OpResult LimitReset(OpBase *opBase);
static OpBase *LimitClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewLimitOp(const ExecutionPlan *plan) {
	OpLimit *op = rm_malloc(sizeof(OpLimit));
	op->consumed = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", LimitInit, LimitConsume, LimitReset, NULL,
				LimitClone, NULL, false, plan);

	return (OpBase *)op;
}

static OpResult LimitInit(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;
	AST *ast = ExecutionPlan_GetAST(opBase->plan);
	op->limit = AST_GetLimit(ast);
	return OP_OK;
}

static Record LimitConsume(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;

	// Have we reached our limit?
	if(op->consumed >= op->limit) return NULL;

	// Consume a single record.
	op->consumed++;
	OpBase *child = op->op.children[0];
	return OpBase_Consume(child);
}

static OpResult LimitReset(OpBase *ctx) {
	OpLimit *limit = (OpLimit *)ctx;
	limit->consumed = 0;
	return OP_OK;
}

static inline OpBase *LimitClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_LIMIT);
	return NewLimitOp(plan);
}
