/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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

OpBase *NewUnwindOp
(
	const ExecutionPlan *plan,
	AR_ExpNode *exp
) {
	OpUnwind *op = rm_malloc(sizeof(OpUnwind));

	op->exp           = exp;
	op->currentRecord = NULL;

	OpBase_Init((OpBase *)op, OPType_UNWIND, "Unwind", UnwindInit,
				UnwindConsume, UnwindReset, NULL, UnwindClone, UnwindFree,
				false, plan);

	op->unwindRecIdx = OpBase_Modifies((OpBase *)op, exp->resolved_name);
	return (OpBase *)op;
}

// Evaluate list expression
// if expression did not returned a list type value
// creates a list with that value
static void _initList
(
	OpUnwind *op
) {	
	if(AR_EXP_IsOperation(op->exp) && strcmp(op->exp->op.f->name, "range") == 0) {
		op->step     = 1;
		op->is_range = true;
		// extracting information from range function
		// extracting: range beginning, range end and optional range step
		SIValue v = AR_EXP_Evaluate(op->exp->op.children[0], op->currentRecord);
		if(SI_TYPE(v) != T_INT64) {
			Error_SITypeMismatch(v, T_INT64);
			ErrorCtx_RaiseRuntimeException(NULL);
		}
		op->from = v.longval;
		v = AR_EXP_Evaluate(op->exp->op.children[1], op->currentRecord);
		if(SI_TYPE(v) != T_INT64) {
			Error_SITypeMismatch(v, T_INT64);
			ErrorCtx_RaiseRuntimeException(NULL);
		}
		op->to = v.longval;
		if(op->exp->op.child_count == 3) {
			v = AR_EXP_Evaluate(op->exp->op.children[2], op->currentRecord);
			if(SI_TYPE(v) != T_INT64) {
				Error_SITypeMismatch(v, T_INT64);
				ErrorCtx_RaiseRuntimeException(NULL);
			}
			op->step = v.longval;
		}
		op->to      += op->step;
		op->current  = op->from;
	} else {
		// null-set the list value to avoid memory errors if evaluation fails
		op->list     = SI_NullVal();
		op->listIdx  = INDEX_NOT_SET;
		op->is_range = false;
		SIValue new_list = AR_EXP_Evaluate(op->exp, op->currentRecord);
		if(SI_TYPE(new_list) == T_ARRAY || SI_TYPE(new_list) == T_NULL) {
			// update the list value
			op->list = new_list;
		} else {
			// create a list of size 1 and initialize it with the input expression value
			op->list = SI_Array(1);
			SIArray_Append(&op->list, new_list);
			SIValue_Free(new_list);
		}
	}
}

static OpResult UnwindInit
(
	OpBase *opBase
) {
	OpUnwind *op = (OpUnwind *) opBase;
	op->currentRecord = OpBase_CreateRecord((OpBase *)op);

	if(op->op.childCount == 0) {
		// no child operation, list must be static
		op->listIdx = 0;
		_initList(op);
	} else {
		// list might depend on data provided by child operation
		op->list = SI_EmptyArray();
		op->listIdx = INDEX_NOT_SET;
	}

	return OP_OK;
}

// try to generate a new value to return
// NULL will be returned if dynamic list is not evaluted (listIdx = INDEX_NOT_SET)
// or in case where the current list is fully consumed
Record _handoff
(
	OpUnwind *op
) {
	if(op->is_range) {
		if(op->current != op->to) {
			Record r = OpBase_CloneRecord(op->currentRecord);
			Record_Add(r, op->unwindRecIdx, SI_LongVal(op->current));
			op->current += op->step;
			return r;
		}
	} else if(op->listIdx < SIArray_Length(op->list)) {
		Record r = OpBase_CloneRecord(op->currentRecord);
		Record_Add(r, op->unwindRecIdx, SIArray_Get(op->list, op->listIdx));
		op->listIdx++;
		return r;
	}
	return NULL;
}

static Record UnwindConsume
(
	OpBase *opBase
) {
	OpUnwind *op = (OpUnwind *)opBase;

	// try to produce data
	Record r = _handoff(op);
	if(r) return r;

	// no child operation to pull data from, we're done
	if(op->op.childCount == 0) return NULL;

	OpBase *child = op->op.children[0];
	// did we managed to get new data?
	if((r = OpBase_Consume(child))) {
		// free current record to accommodate new record
		OpBase_DeleteRecord(op->currentRecord);
		op->currentRecord = r;
		// free old list
		SIValue_Free(op->list);

		// reset index and set list
		op->listIdx = 0;
		_initList(op);
	}

	return _handoff(op);
}

static OpResult UnwindReset
(
	OpBase *ctx
) {
	OpUnwind *op = (OpUnwind *)ctx;
	// static should reset index to 0
	if(op->op.childCount == 0) op->listIdx = 0;
	// dynamic should set index to UINT_MAX, to force refetching of data
	else op->listIdx = INDEX_NOT_SET;
	return OP_OK;
}

static inline OpBase *UnwindClone
(
	const ExecutionPlan *plan,
	const OpBase *opBase
) {
	ASSERT(opBase->type == OPType_UNWIND);
	OpUnwind *op = (OpUnwind *)opBase;
	return NewUnwindOp(plan, AR_EXP_Clone(op->exp));
}

static void UnwindFree
(
	OpBase *ctx
) {
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
