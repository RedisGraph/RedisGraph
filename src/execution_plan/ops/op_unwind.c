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

OpBase *NewUnwindOp(uint record_idx, AR_ExpNode *exp) {
	OpUnwind *unwind = malloc(sizeof(OpUnwind));

	unwind->exp = exp;
	unwind->list = SI_NullVal();
	unwind->currentRecord = NULL;
	unwind->listIdx = INDEX_NOT_SET;
	unwind->unwindRecIdx = record_idx;

	// Set our Op operations
	OpBase_Init(&unwind->op);
	unwind->op.name = "Unwind";
	unwind->op.type = OPType_UNWIND;
	unwind->op.consume = UnwindConsume;
	unwind->op.init = UnwindInit;
	unwind->op.reset = UnwindReset;
	unwind->op.free = UnwindFree;

	// Handle introduced entity
	unwind->op.modifies = array_new(uint, 1);
	unwind->op.modifies = array_append(unwind->op.modifies, record_idx);

	return (OpBase *)unwind;
}

OpResult UnwindInit(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *) opBase;
	op->currentRecord = Record_New(1);

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

Record UnwindConsume(OpBase *opBase) {
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

OpResult UnwindReset(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	// Static should reset index to 0.
	if(op->op.childCount == 0) op->listIdx = 0;
	// Dynamic should set index to UINT_MAX, to force refetching of data.
	else op->listIdx = INDEX_NOT_SET;
	return OP_OK;
}

void UnwindFree(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	SIValue_Free(&op->list);
	if(op->exp) AR_EXP_Free(op->exp);
	if(op->currentRecord) Record_Free(op->currentRecord);
}
