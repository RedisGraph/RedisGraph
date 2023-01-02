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

    op->supplier = NULL;
	op->first_embedded = NULL;
	op->exp = exp;
    op->argument = NULL;
	op->list = SI_NullVal();
	op->currentRecord = NULL;
	op->listIdx = INDEX_NOT_SET;

    OpBase_Init((OpBase *)op, OPType_FOREACH, "Foreach", ForeachInit, ForeachConsume,
				ForeachReset, NULL, ForeachClone, ForeachFree, false, plan);

	op->foreachRecIdx = OpBase_Modifies((OpBase *)op, exp->resolved_name);

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

	// if number of children is larger than one --> we have a supplier and 
	// a consumer. Otherwise, we have only a consumer.
	if(op->op.childCount > 1) {
		op->supplier = op->op.children[0];
		op->first_embedded = op->op.children[1];
	}
	else {
		op->first_embedded = op->op.children[0];
	}

	// update argument operation to be the deepest child of the first embedded
	// child
	OpBase *argument = op->first_embedded;
	while(argument->childCount > 0) {
		argument = argument->children[0];
	}
	op->argument = (Argument *)argument;


    // Initialize struct components to initial values (list etc.).
    op->currentRecord = OpBase_CreateRecord((OpBase *)op);

	if(op->op.childCount == 1) {
		// No supplier operation, list must be static.
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
 * NULL will be returned if dynamic list is not evaluated (listIdx = INDEX_NOT_SET)
 * or in case where the current list is fully consumed. */
static Record _handoff(OpForeach *op) {
	// If there is a new value ready, return it.
	if(op->listIdx < SIArray_Length(op->list)) {
        Record r = OpBase_CloneRecord(op->currentRecord);
		Record_Add(r, op->foreachRecIdx, SIArray_Get(op->list, op->listIdx));
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

    Record r;
    OpBase *supplier       = op->supplier;
    Argument *argument     = op->argument;
	OpBase *first_embedded = op->first_embedded;

    // Manually insert next list-component to the Argument operation, then 
	// call the consume function on the first_embedded operation.

	r = _handoff(op);
	if(r == NULL) {
		
		// try pulling a records from supplier child. If can't -> We're done
		if(op->op.childCount == 1 || !(r = OpBase_Consume(supplier))) {
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
		
		// set next record for planting in argument
		r = _handoff(op);
	}

	// Take list apart, feeding the argument each component (added to the 
	// original record received), until the list is fully traversed
	do {
		// plant the record in the argument operation
		Argument_AddRecord(argument, r);

		// call consume function on first embedded child.
		// ignore record returned from it.
		while(OpBase_Consume(first_embedded) != NULL) {}
		r = _handoff(op);
	} while(r != NULL);
	OpBase_PropagateReset(first_embedded);


	return op->currentRecord;
}

static OpResult ForeachReset
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;
	// index should reset to 0 in static case.
	if(op->op.childCount == 1) {
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
