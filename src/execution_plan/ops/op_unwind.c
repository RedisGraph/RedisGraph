/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../datatypes/array.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "limits.h"

#define INDEX_NOT_SET UINT_MAX

/* Forward declarations. */
static OpResult UnwindInit(OpBase *opBase);
static Record UnwindConsume(OpBase *opBase);
static OpResult UnwindReset(OpBase *opBase);
static void UnwindFree(OpBase *opBase);

OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp) {
	OpUnwind *op = malloc(sizeof(OpUnwind));

	op->exp = exp;
	op->firstcall = true;
	op->list = SI_NullVal();
	op->currentRecord = NULL;
	op->listIdx = INDEX_NOT_SET;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", UnwindInit, UnwindConsume,
				UnwindReset, NULL, UnwindFree, plan);

	op->unwindRecIdx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
	return (OpBase *)op;
}

static OpResult UnwindInit(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *) opBase;

	if(op->op.childCount == 0) {
		// No child operation, list must be static.
		op->listIdx = 0;
		op->list = AR_EXP_Evaluate(op->exp, op->currentRecord);
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
	if(op->currentRecord && op->listIdx < SIArray_Length(op->list)) {
		Record r = Record_Clone(op->currentRecord);
		Record_AddScalar(r, op->unwindRecIdx, SIArray_Get(op->list, op->listIdx));
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

	if(op->currentRecord) Record_Free(op->currentRecord);
	op->currentRecord = NULL;

	// Make sure we have a record to work with.
	if(op->op.childCount > 0) {
		OpBase *child = op->op.children[0];
		op->currentRecord = OpBase_Consume(child);
	} else if(op->firstcall) {
		op->firstcall = false;
		// No operations above us, we're a TAP.
		op->currentRecord = OpBase_CreateRecord((OpBase *)op);
	}

	// Did we managed to get a record?
	if(op->currentRecord) {
		op->listIdx = 0;            // Reset index and set list.
		SIValue_Free(&op->list);    // Free old list & current record.
		op->list = AR_EXP_Evaluate(op->exp, op->currentRecord);
		assert(op->list.type == T_ARRAY);
	}

	return _handoff(op);
}

static OpResult UnwindReset(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	// Static should reset index to 0.
	op->listIdx = 0;
	SIValue_Free(&op->list);
	op->list = SI_EmptyArray();

	op->firstcall = true;
	if(op->currentRecord) {
		Record_Free(op->currentRecord);
		op->currentRecord = NULL;
	}

	return OP_OK;
}

static void UnwindFree(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	SIValue_Free(&op->list);
	op->list = SI_NullVal();

	if(op->exp) {
		AR_EXP_Free(op->exp);
		op->exp = NULL;
	}

	if(op->currentRecord) {
		Record_Free(op->currentRecord);
		op->currentRecord = NULL;
	}
}
