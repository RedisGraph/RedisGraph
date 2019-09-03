/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

/* Forward declarations */
static Record Consume(OpBase *opBase);
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static OpResult Init(OpBase *opBase);

static AR_ExpNode **_getOrderExpressions(OpBase *op) {
	if(op == NULL) return NULL;
	// No need to look further if we haven't encountered a sort operation
	// before a project/aggregate op
	if(op->type == OPType_PROJECT || op->type == OPType_AGGREGATE) return NULL;

	if(op->type == OPType_SORT) {
		OpSort *sort = (OpSort *)op;
		return sort->expressions;
	}
	return _getOrderExpressions(op->parent);
}

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	AST *ast = AST_GetFromTLS();
	OpProject *op = malloc(sizeof(OpProject));
	op->ast = ast;
	op->exps = exps;
	op->exp_count = array_len(exps);
	op->order_exps = NULL;
	op->order_exp_count = 0;
	op->singleResponse = false;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", Init, Consume, Reset, NULL, Free, plan);

	assert("Set modifiers");
	for(int i = 0; i < op->exp_count; i++) {
		char *str = NULL;
		AR_EXP_ToString(exps[i], &str);
		int rec_idx = OpBase_Modifies(op, str);


	}

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	OpProject *op = (OpProject *)opBase;
	AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
	if(order_exps) {
		op->order_exps = order_exps;
		op->order_exp_count = array_len(order_exps);
	}

	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	OpProject *op = (OpProject *)opBase;
	Record r = NULL;

	if(op->op.childCount) {
		OpBase *child = op->op.children[0];
		r = OpBase_Consume(child);
		if(!r) return NULL;
	} else {
		// QUERY: RETURN 1+2
		// Return a single record followed by NULL
		// on the second call.

		if(op->singleResponse) return NULL;
		op->singleResponse = true;
		// Fake empty record.
		r = OpBase_CreateRecord((OpBase *)op);
	}

	Record projection = OpBase_CreateRecord((OpBase *)op);
	int rec_idx = 0;
	for(unsigned short i = 0; i < op->exp_count; i++) {
		SIValue v = AR_EXP_Evaluate(op->exps[i], r);
		Record_Add(projection, rec_idx, v);
		rec_idx++;
	}

	// Project Order expressions.
	for(unsigned short i = 0; i < op->order_exp_count; i++) {
		SIValue v = AR_EXP_Evaluate(op->order_exps[i], r);
		Record_Add(projection, rec_idx, v);
		rec_idx++;
	}

	Record_Free(r);
	return projection;
}

static OpResult Reset(OpBase *ctx) {
	return OP_OK;
}

static void Free(OpBase *ctx) {
	OpProject *op = (OpProject *)ctx;
	// TODO These expressions are typically freed as part of
	// _ExecutionPlanSegment_Free, but this forms a leak in scenarios
	// like the ReduceCount optimization.
	// if (op->exps) array_free(op->exps);
}
