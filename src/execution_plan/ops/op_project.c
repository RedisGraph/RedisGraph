/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_project.h"
#include "op_sort.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"

/* Forward declarations. */
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

static AR_ExpNode **_getOrderExpressions(OpBase *op) {
	if(op == NULL) return NULL;
	// No need to look further if we haven't encountered a sort operation
	// before a project/aggregate op
	if(op->type == OPType_PROJECT || op->type == OPType_AGGREGATE) return NULL;

	if(op->type == OPType_SORT) {
		OpSort *sort = (OpSort *)op;
		return sort->exps;
	}
	return _getOrderExpressions(op->parent);
}

OpBase *NewProjectOp(const ExecutionPlan *plan, AR_ExpNode **exps) {
	AST *ast = QueryCtx_GetAST();
	OpProject *op = malloc(sizeof(OpProject));
	op->ast = ast;
	op->exps = exps;
	op->exp_count = array_len(exps);
	op->order_exps = NULL;
	op->order_exp_count = 0;
	op->singleResponse = false;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_PROJECT, "Project", Init, Consume, Reset, NULL, Free, plan);

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	OpProject *op = (OpProject *)opBase;
	for(uint i = 0; i < op->exp_count; i ++) {
		// The projected record will associate values with their resolved name
		// to ensure that space is allocated for each entry.
		OpBase_Modifies(opBase, op->exps[i]->resolved_name);
	}
	AR_ExpNode **order_exps = _getOrderExpressions(opBase->parent);
	if(order_exps) {
		op->order_exps = order_exps;
		op->order_exp_count = array_len(order_exps);

		for(uint i = 0; i < op->order_exp_count; i ++) {
			OpBase_Modifies(opBase, op->order_exps[i]->resolved_name);
		}
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
		r = OpBase_CreateRecord(opBase);
	}

	Record projection = OpBase_CreateRecord(opBase);
	// Track the inherited Record and the newly-allocated Record so that they may be freed if execution fails.
	OpBase_AddVolatileRecord(opBase, r);
	OpBase_AddVolatileRecord(opBase, projection);

	for(unsigned short i = 0; i < op->exp_count; i++) {
		AR_ExpNode *exp = op->exps[i];
		SIValue v = AR_EXP_Evaluate(exp, r);
		int rec_idx = Record_GetEntryIdx(projection, exp->resolved_name);
		/* Persisting a value is only necessary here if 'v' refers to a scalar held in Record 'r'.
		 * Graph entities don't need to be persisted here as Record_Add will copy them internally.
		 * The RETURN projection here requires persistence:
		 * MATCH (a) WITH toUpper(a.name) AS e RETURN e
		 * TODO This is a rare case; the logic of when to persist can be improved.  */
		if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
		Record_Add(projection, rec_idx, v);
	}

	// Project Order expressions.
	for(unsigned short i = 0; i < op->order_exp_count; i++) {
		AR_ExpNode *order_exp = op->order_exps[i];
		SIValue v = AR_EXP_Evaluate(order_exp, r);
		int rec_idx = Record_GetEntryIdx(projection, order_exp->resolved_name);
		// TODO persisting here can be improved as described above.
		if(!(v.type & SI_GRAPHENTITY)) SIValue_Persist(&v);
		Record_Add(projection, rec_idx, v);
	}

	Record_Free(r);
	OpBase_RemoveVolatileRecords(opBase); // No exceptions encountered, Records are not dangling.
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

