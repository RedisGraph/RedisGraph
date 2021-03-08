/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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

// forward declarations
static OpResult UnwindInit(OpBase *opBase);
static RecordBatch UnwindConsume(OpBase *opBase);
static OpResult UnwindReset(OpBase *opBase);
static OpBase *UnwindClone(const ExecutionPlan *plan, const OpBase *opBase);
static void UnwindFree(OpBase *opBase);

OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp) {
	OpUnwind *op = rm_malloc(sizeof(OpUnwind));

	op->exp            =  exp;
	op->list           =  SI_NullVal();
	op->listIdx        =  INDEX_NOT_SET;
	op->in_batch       =  NULL;
	op->batchIdx       =  0;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", UnwindInit, UnwindConsume,
				UnwindReset, NULL, UnwindClone, UnwindFree, false, plan);

	op->unwindRecIdx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
	return (OpBase *)op;
}

/* evaluate list expression, raise runtime exception
 * if expression did not returned a list type value */
static void _initList(OpUnwind *op) {
	// Null-set the list value to avoid memory errors if evaluation fails
	op->list = SI_NullVal(); 
	Record r = op->in_batch[op->batchIdx];
	SIValue new_list = AR_EXP_Evaluate(op->exp, r);
	if(SI_TYPE(new_list) != T_ARRAY) {
		Error_SITypeMismatch(new_list, T_ARRAY);
		SIValue_Free(new_list);
		ErrorCtx_RaiseRuntimeException(NULL);
	}

	// update the list value
	op->listIdx = 0;
	op->list = new_list;
}

static OpResult UnwindInit(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *) opBase;

	if(op->op.childCount == 0) {
		// create fake input record batch
		Record r = OpBase_CreateRecord(opBase);
		RecordBatch batch = RecordBatch_New(1);
		RecordBatch_Add(&batch, r);
		op->in_batch = batch;
	}

	return OP_OK;
}

/* try to generate a new value to return NULL will be returned
 * if dynamic list is not evaluted (listIdx = INDEX_NOT_SET)
 * or in case where the current list is fully consumed */
void _populateBatch(OpUnwind *op) {
	OpBase       *opBase     =  (OpBase *)op;
	RecordBatch  batch       =  op->in_batch;
	uint         batch_size  =  RecordBatch_Len(batch);

	// for each record in input batch
	for(; op->batchIdx < batch_size && OP_BATCH_HAS_ROOM(); op->batchIdx++) {

		// evaluate list expression if needed
		if(SI_TYPE(op->list) & T_NULL) _initList(op);

		// determine number of items to extract from list
		uint  start          =  op->listIdx;
		uint  remaining      =  SIArray_Length(op->list) - op->listIdx;
		uint  room_in_batch  =  EXEC_PLAN_BATCH_SIZE - OP_BATCH_LEN();
		uint  item_count     =  MIN(room_in_batch, remaining);
		uint  end            =  start + item_count;

		// extract count elements from list to out batch
		for(uint i = start; i < end; i++) {
			Record r = OpBase_CloneRecord(batch[op->batchIdx]);
			Record_Add(r, op->unwindRecIdx, SIArray_Get(op->list, i));
			OP_BATCH_ADD(r);
		}

		op->listIdx += item_count; // update list index position

		// free list if fully consumed
		if(SIArray_Length(op->list) == op->listIdx) {
			op->listIdx = 0;
			SIValue_Free(op->list);
			op->list = SI_NullVal();
			//OpBase_DeleteRecord(op->batch[op->batchIdx]);
			//op->batch[op->batchIdx] = NULL;
		}
	}
}

static RecordBatch UnwindConsume(OpBase *opBase) {
	OpUnwind *op = (OpUnwind *)opBase;

	OP_BATCH_CLEAR();

	// try to produce data
	_populateBatch(op);
	if(!OP_BATCH_EMPTY()) goto emit;

	// no child operation to pull data from, we're done
	if(op->op.childCount == 0) goto emit;

	OpBase *child = op->op.children[0];
	// as long as child isn't depleted and there's room in output batch
	do {
		op->batchIdx = 0;
		op->in_batch = OpBase_Consume(child);
		_populateBatch(op);
	} while(!RecordBatch_Empty(op->in_batch) && OP_BATCH_HAS_ROOM());

emit:
	OP_BATCH_EMIT();
}

static OpResult UnwindReset(OpBase *ctx) {
	OpUnwind *op = (OpUnwind *)ctx;
	// static should reset index to 0
	if(op->op.childCount == 0) op->listIdx = 0;
	// dynamic should set index to UINT_MAX, to force refetching of data
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

	//if(op->batch) {
	//	uint batch_size = RecordBatch_Len(op->batch);
	//	for(uint i = 0; i < batch_size; i++) {
	//		Record r = op->batch[i];
	//		if(r != NULL) OpBase_DeleteRecord(op->batch[i]);
	//	}
	//	RecordBatch_Free(op->batch);
	//	op->batch = NULL;
	//}
}

