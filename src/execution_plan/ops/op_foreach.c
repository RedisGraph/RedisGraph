/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "op_foreach.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/array.h"
#include "../../arithmetic/arithmetic_expression.h"

#define INDEX_NOT_SET UINT_MAX

/* Forward declarations. */
static OpResult ForeachInit(OpBase *opBase);
static Record ForeachConsume(OpBase *opBase);
static OpResult ForeachReset(OpBase *opBase);
static OpBase *ForeachClone(const ExecutionPlan *plan, const OpBase *opBase);
static void ForeachFree(OpBase *opBase);

/* Creates a new Foreach operation */
OpBase *NewForeachOp
(
    const ExecutionPlan *plan,  // execution plan
    AR_ExpNode *exp             // arithmetic expression node
) {
    OpForeach *op = rm_calloc(1, sizeof(OpForeach));
    // initialize struct components to NULL
    
    op->argument = NULL;
    op->exp = exp;
	op->list = SI_NullVal();
	op->currentRecord = NULL;
	op->listIdx = INDEX_NOT_SET;

    OpBase_Init((OpBase *)op, OPType_FOREACH, "Foreach", ForeachInit, ForeachConsume,
				ForeachReset, NULL, ForeachClone, ForeachFree, true, plan);
	
	return (OpBase *)op;
}

/* Evaluate list expression, 
 * if expression did not returned a list type value, creates a list with that value. */
static void _initList(OpForeach *op) {
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

static OpResult ForeachInit
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;

    // Initialize struct components to initial values (list etc.).
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
static Record _handoff(OpForeach *op) {
	// If there is a new value ready, return it.
	if(op->listIdx < SIArray_Length(op->list)) {
        Record r = OpBase_CloneRecord(op->currentRecord);
		// TODO: Change from add (?)
		Record_Add(r, 0, SIArray_Get(op->list, op->listIdx));
		op->listIdx++;
		return r;
	}
	return NULL;
}

static Record ForeachConsume
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;
    OpBase *supplier = op->op.children[0];
	OpBase *first_embedded = op->op.children[1];
    Argument *argument = op->argument;
    Record r;

    // Manually insert next list-component to the Argument operation, then 
	// call the consume function on the first_embedded operation.

	while(true) {
		// try extracting a new component of the list
		Record r = _handoff(op);
		if(!r) {
			// return current record if this is not the first time entering.
			// (pass record on to next operation)
			// TBD

			if(op->op.childCount == 0 || !(r = OpBase_Consume(supplier))) {
				return NULL;
			}

			// Free current record to accommodate new record.
			OpBase_DeleteRecord(op->currentRecord);
			op->currentRecord = r;
			// Free old list.
			SIValue_Free(op->list);

			// Reset index and set list.
			op->listIdx = 0;
			_initList(op);
		}
		
		// plant the record in the argument operation
		argument->r = r;

		// call consume function on second child.
		// ignore record returned from it.
		OpBase_Consume(first_embedded);
	}

	return r;
}

static OpResult ForeachReset
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;
	// Static should reset index to 0.
	if(op->op.childCount == 0) {
		op->listIdx = 0;
	}
	// Dynamic should set index to UINT_MAX, to force refetching of data.
	else op->listIdx = INDEX_NOT_SET;
	return OP_OK;
}

static OpBase *ForeachClone
(
    const ExecutionPlan *plan,  // plan
    const OpBase *opBase        // operation
) {
    ASSERT(opBase->type == OPType_FOREACH);
	OpForeach *op = (OpForeach *)opBase;
	return NewForeachOp(plan, AR_EXP_Clone(op->exp));
}

static void ForeachFree
(
    OpBase *opBase
) {
    OpForeach *op = (OpForeach *)opBase;
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
