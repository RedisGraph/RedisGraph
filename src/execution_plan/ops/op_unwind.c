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
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp) {
	OpUnwind *op = malloc(sizeof(OpUnwind));

	op->exp = exp;
	op->list = SI_NullVal();
	op->currentRecord = NULL;
	op->listIdx = INDEX_NOT_SET;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", Init, Consume, Reset, NULL, Free, plan);

	op->unwindRecIdx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *) opBase;
	op->currentRecord = OpBase_CreateRecord((OpBase *)op);

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
	if(op->listIdx < SIArray_Length(op->list)) {
		Record r = Record_Clone(op->currentRecord);
		Record_AddScalar(r, op->unwindRecIdx, SIArray_Get(op->list, op->listIdx));
		op->listIdx++;
		return r;
	}
	return NULL;
}

static Record Consume(OpBase *opBase) {
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
		Record_Free(op->currentRecord);
		op->currentRecord = r;
		// Free old list.
		SIValue_Free(&op->list);

		// Reset index and set list.
		op->listIdx = 0;
		op->list = AR_EXP_Evaluate(op->exp, r);
		assert(op->list.type == T_ARRAY);
	}

	return _handoff(op);
}

static OpResult Reset(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	// Static should reset index to 0.
	if(op->op.childCount == 0) op->listIdx = 0;
	// Dynamic should set index to UINT_MAX, to force refetching of data.
	else op->listIdx = INDEX_NOT_SET;
	return OP_OK;
}

static void Free(OpBase *ctx) {
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

