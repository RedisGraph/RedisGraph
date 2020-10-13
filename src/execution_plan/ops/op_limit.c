/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_limit.h"
#include "../../RG.h"
#include "../../query_ctx.h"
#include "../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static Record LimitConsume(OpBase *opBase);
static OpResult LimitReset(OpBase *opBase);
static void LimitFree(OpBase *opBase);
static OpBase *LimitClone(const ExecutionPlan *plan, const OpBase *opBase);

static void _eval_limit(OpLimit *op, AR_ExpNode *limit_exp) {
	// make a copy of the original expression, this is required as in the case
	// of parametrised limit: "LIMIT $L" evaluating the expression will
	// modify it: replacing the parameter with a constant, as a result clones
	// of this operation will contain a constant instead of a parameter.
	op->limit_exp = AR_EXP_Clone(limit_exp);

	// evaluate using the original expression
	SIValue l = AR_EXP_Evaluate(limit_exp, NULL);

	// validate limit is numeric
	if(SI_TYPE(l) != T_INT64) {
		QueryCtx_SetError("Limit operates only on non-negative integers");
	}

	op->limit = SI_GET_NUMERIC(l);

	// free original expression
	// AR_EXP_Free(limit_exp);
}

OpBase *NewLimitOp(const ExecutionPlan *plan, AR_ExpNode *limit_exp) {
	// validate inputs
	ASSERT(plan != NULL);
	ASSERT(limit_exp != NULL);

	OpLimit *op = rm_malloc(sizeof(OpLimit));
	op->limit = 0;
	op->consumed = 0;
	op->limit_exp = NULL;

	_eval_limit(op, limit_exp);

	// set operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", NULL, LimitConsume, LimitReset, NULL,
				LimitClone, LimitFree, false, plan);

	return (OpBase *)op;
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
	ASSERT(opBase->type == OPType_LIMIT);

	OpLimit *op = (OpLimit *)opBase;
	return NewLimitOp(plan, op->limit_exp);
}

static void LimitFree(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;

	if(op->limit_exp) {
		AR_EXP_Free(op->limit_exp);
		op->limit_exp = NULL;
	}
}

