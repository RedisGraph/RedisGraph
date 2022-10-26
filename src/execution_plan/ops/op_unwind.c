/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_unwind.h"
#include "../../errors.h"
#include "../../query_ctx.h"
#include "../../datatypes/array.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "limits.h"

#define INDEX_NOT_SET UINT_MAX

/* Forward declarations. */
static OpResult UnwindInit(OpBase *opBase);
static Record UnwindConsume(OpBase *opBase);
static OpResult UnwindReset(OpBase *opBase);
static OpBase *UnwindClone(const ExecutionPlan *plan, const OpBase *opBase);
static void UnwindFree(OpBase *opBase);

OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp) {
	OpUnwind *op = rm_malloc(sizeof(OpUnwind));

	op->exp = exp;
	op->list = SI_NullVal();
	op->currentRecord = NULL;
	op->listIdx = INDEX_NOT_SET;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", UnwindInit, UnwindConsume,
				UnwindReset, NULL, UnwindClone, UnwindFree, false, plan);

	op->unwindRecIdx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
	return (OpBase *)op;
}

/* Evaluate list expression, 
 * if expression did not returned a list type value, creates a list with that value. */
static void _initList(OpUnwind *op) {
	op->list = SI_NullVal(); // Null-set the list value to avoid memory errors if evaluation fails.
	SIValue new_list = AR_EXP_Evaluate(op->exp, op->currentRecord);
	if(SI_TYPE(new_list) == T_ARRAY || SI_TYPE(new_list) == T_NULL) {
		// Update the list value.
		op->list = new_list;
	} else {
		// Create a list of size 1 and initialize it with the input expression value
		op->list = SI_Array(1);
		SIArray_Append(&op->list, new_list);
		SIValue_Free(new_list);
	}
}

static OpResult UnwindInit(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *) opBase;
	op->currentRecord = OpBase_CreateRecord((OpBase *)op);

	if(op->op.childCount == 0) {
		// No child operation, list must be static.
		op->listIdx = 0;
		_initList(op);
	} else {
		// List might depend on data provided by child operation.
		op->list = SI_EmptyArray();
		op->listIdx = INDEX_NOT_SET;
	}

	return OP_OK;
}

/* Try to generate a new value to return
 * NULL will be returned if dynamic list is not evaluted (listIdx = INDEX_NOT_SET)
 * or in case where the current list is fully consumed. */
Record _handoff(OpUnwind *op) {
	// If there is a new value ready, return it.
	if(op->listIdx < SIArray_Length(op->list)) {
		Record r = OpBase_CloneRecord(op->currentRecord);
		Record_Add(r, op->unwindRecIdx, SIArray_Get(op->list, op->listIdx));
		op->listIdx++;
		return r;
	}
	return NULL;
}

static Record UnwindConsume(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;

	// Try to produce data.
	Record r = _handoff(op);
	if(r) return r;

	// No child operation to pull data from, we're done.
	if(op->op.childCount == 0) return NULL;

	OpBase *child = op->op.children[0];
	// Did we managed to get new data?
	if((r = OpBase_Consume(child))) {
		// Free current record to accommodate new record.
		OpBase_DeleteRecord(op->currentRecord);
		op->currentRecord = r;
		// Free old list.
		SIValue_Free(op->list);

		// Reset index and set list.
		op->listIdx = 0;
		_initList(op);
	}

	return _handoff(op);
}

static OpResult UnwindReset(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	// Static should reset index to 0.
	if(op->op.childCount == 0) op->listIdx = 0;
	// Dynamic should set index to UINT_MAX, to force refetching of data.
	else op->listIdx = INDEX_NOT_SET;
	return OP_OK;
}

static inline OpBase *UnwindClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_UNWIND);
	OpUnwind *op = (OpUnwind *)opBase;
	return NewUnwindOp(plan, AR_EXP_Clone(op->exp));
}

static void UnwindFree(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	SIValue_Free(op->list);
	op->list = SI_NullVal();

	if(op->exp) {
		AR_EXP_Free(op->exp);
		op->exp = NULL;
	}

	if(op->currentRecord) {
		OpBase_DeleteRecord(op->currentRecord);
		op->currentRecord = NULL;
	}
}

