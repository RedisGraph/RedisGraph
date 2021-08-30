/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "limits.h"
#include "op_unwind.h"
#include "../../../../errors.h"
#include "../../../../query_ctx.h"
#include "../../../../datatypes/array.h"
#include "../../../../arithmetic/arithmetic_expression.h"

#define INDEX_NOT_SET UINT_MAX

/* Forward declarations. */
static RT_OpResult UnwindInit(RT_OpBase *opBase);
static Record UnwindConsume(RT_OpBase *opBase);
static RT_OpResult UnwindReset(RT_OpBase *opBase);
static RT_OpBase *UnwindClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void UnwindFree(RT_OpBase *opBase);

RT_OpBase *RT_NewUnwindOp(const RT_ExecutionPlan *plan, const OpUnwind *op_desc) {
	RT_OpUnwind *op = rm_malloc(sizeof(RT_OpUnwind));
	op->op_desc = op_desc;
	op->list = SI_NullVal();
	op->currentRecord = NULL;
	op->listIdx = INDEX_NOT_SET;

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_UNWIND, UnwindInit, UnwindConsume,
				UnwindReset, UnwindClone, UnwindFree, false, plan);

	RT_OpBase_Aware((RT_OpBase *)op, op_desc->exp->resolved_name, &op->unwindRecIdx);
	
	return (RT_OpBase *)op;
}

/* Evaluate list expression, raise runtime exception
 * if expression did not returned a list type value. */
static void _initList(RT_OpUnwind *op) {
	op->list = SI_NullVal(); // Null-set the list value to avoid memory errors if evaluation fails.
	SIValue new_list = AR_EXP_Evaluate(op->op_desc->exp, op->currentRecord);
	if(SI_TYPE(new_list) != T_ARRAY) {
		Error_SITypeMismatch(new_list, T_ARRAY);
		SIValue_Free(new_list);
		ErrorCtx_RaiseRuntimeException(NULL);
	}
	// Update the list value.
	op->list = new_list;
}

static RT_OpResult UnwindInit(RT_OpBase *opBase) {
	RT_OpUnwind *op = (RT_OpUnwind *) opBase;
	op->currentRecord = RT_OpBase_CreateRecord((RT_OpBase *)op);

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
Record _handoff(RT_OpUnwind *op) {
	// If there is a new value ready, return it.
	if(op->listIdx < SIArray_Length(op->list)) {
		Record r = RT_OpBase_CloneRecord(op->currentRecord);
		Record_Add(r, op->unwindRecIdx, SIArray_Get(op->list, op->listIdx));
		op->listIdx++;
		return r;
	}
	return NULL;
}

static Record UnwindConsume(RT_OpBase *opBase) {
	RT_OpUnwind *op = (RT_OpUnwind *)opBase;

	// Try to produce data.
	Record r = _handoff(op);
	if(r) return r;

	// No child operation to pull data from, we're done.
	if(op->op.childCount == 0) return NULL;

	RT_OpBase *child = op->op.children[0];
	// Did we managed to get new data?
	if((r = RT_OpBase_Consume(child))) {
		// Free current record to accommodate new record.
		RT_OpBase_DeleteRecord(op->currentRecord);
		op->currentRecord = r;
		// Free old list.
		SIValue_Free(op->list);

		// Reset index and set list.
		op->listIdx = 0;
		_initList(op);
	}

	return _handoff(op);
}

static RT_OpResult UnwindReset(RT_OpBase *ctx) {
	RT_OpUnwind *op = (RT_OpUnwind *)ctx;
	// Static should reset index to 0.
	if(op->op.childCount == 0) op->listIdx = 0;
	// Dynamic should set index to UINT_MAX, to force refetching of data.
	else op->listIdx = INDEX_NOT_SET;
	return OP_OK;
}

static inline RT_OpBase *UnwindClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_UNWIND);
	RT_OpUnwind *op = (RT_OpUnwind *)opBase;
	return RT_NewUnwindOp(plan, op->op_desc);
}

static void UnwindFree(RT_OpBase *ctx) {
	RT_OpUnwind *op = (RT_OpUnwind *)ctx;
	SIValue_Free(op->list);
	op->list = SI_NullVal();

	if(op->currentRecord) {
		RT_OpBase_DeleteRecord(op->currentRecord);
		op->currentRecord = NULL;
	}
}
