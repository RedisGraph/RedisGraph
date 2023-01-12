/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "op_foreach.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../src/value.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/array.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "op_argument_list.h"

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
    const ExecutionPlan *plan  // execution plan
) {
    OpForeach *op = rm_calloc(1, sizeof(OpForeach));

    op->supplier = NULL;
	op->first_embedded = NULL;
    op->argument = NULL;
	op->records = NULL;
	op->first = true;

    OpBase_Init((OpBase *)op, OPType_FOREACH, "Foreach", ForeachInit, ForeachConsume,
				ForeachReset, NULL, ForeachClone, NULL, false, plan);

	// set the record index in which the final array will be written to.
	op->recIdx = OpBase_Modifies((OpBase *)op, "array_holder");

	return (OpBase *)op;
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

    return OP_OK;
}

static Record _handoff(OpForeach *op) {
	// if there is a record to return, return it
	Record r = NULL;
	if(array_len(op->records)) {
		r = array_pop(op->records);
	}
	return r;
}

static Record ForeachConsume
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;

	if(!op->first) {
		return _handoff(op);
	}

	// construct an array (SI_Value) containing the records passed by the supplier
	// this happens ONCE
	op->records = array_new(Record, 1);
	// SIValue recs = SI_Array(1);
	// SIArray_Append(&recs, SI_PtrVal((void *) op->records));

	Record r = NULL;
	if(op->supplier) {
		while((r = OpBase_Consume(op->supplier))) {
			// persist scalars from previous ops, which may be freed before the
			// records are handed of
			Record_PersistScalars(r);
			array_append(op->records, r);
			// SIArray_Append(&recs, SI_PtrVal((void *) &r));
		}
	} else {
		// static list, just wrap it in a list ONCE
		// The inner Unwind operation will not need this record, just send
		// and empty one
		r = OpBase_CreateRecord((OpBase *)op);
		array_append(op->records, r);

		op->first = false;
	}

	// plant the list of arguments in argument_list operation
	Record *clone;
	array_clone(clone, op->records);
	ArgumentList_AddRecordList(op->argument, clone);

	// call consume on first_embedded op. The result is thrown away.
	while(OpBase_Consume(op->first_embedded)) {};
	
	// mark that the aggregation has occurred, so it won't occur again
	op->first = false;

	return _handoff(op);
}

static OpResult ForeachReset
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;
	op->first = false;

	return OP_OK;
}

static OpBase *ForeachClone
(
    const ExecutionPlan *plan,  // plan
    const OpBase *opBase        // operation
) {
    ASSERT(opBase->type == OPType_FOREACH);
	return NewForeachOp(plan);
}

static void ForeachFree
(
	OpBase *op
) {
	OpForeach *_op = (OpForeach *) op;
	if(_op->records) {
		array_free(_op->records);
	}
}
