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
static void LimitFree(OpBase *opBase);

OpBase *NewLimitOp(const ExecutionPlan *plan, AR_ExpNode *limit_expr) {
	OpLimit *op = rm_malloc(sizeof(OpLimit));
	op->limit_expr = limit_expr;
	op->consumed = 0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_LIMIT, "Limit", LimitInit, LimitConsume, LimitReset, NULL,
				LimitClone, LimitFree, false, plan);

	return (OpBase *)op;
}

static OpResult LimitInit(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;
	op->limit = UINT_MAX;
	if(op->limit_expr) {
		SIValue limit_value =  AR_EXP_Evaluate(op->limit_expr, NULL);
		if(SI_TYPE(limit_value) != T_INT64) {
			char *error;
			asprintf(&error, "LIMIT specified value of invalid type, must be a positive integer");
			QueryCtx_SetError(error); // Set the query-level error.
			QueryCtx_RaiseRuntimeException();
			op->limit = 0;
			return OP_ERR;
		}
		op->limit = limit_value.longval;
	}
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
	OpLimit *op = (OpLimit *)opBase;
	return NewLimitOp(plan, AR_EXP_Clone(op->limit_expr));
}

static inline void LimitFree(OpBase *opBase) {
	OpLimit *op = (OpLimit *)opBase;
	if(op->limit_expr) {
		AR_EXP_Free(op->limit_expr);
		op->limit_expr = NULL;
	}
}
