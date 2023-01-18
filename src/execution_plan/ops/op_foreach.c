/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_foreach.h"

// Forward declarations
static void ForeachFree(OpBase *opBase);
static OpResult ForeachInit(OpBase *opBase);
static Record ForeachConsume(OpBase *opBase);
static OpResult ForeachReset(OpBase *opBase);
static OpBase *ForeachClone(const ExecutionPlan *plan, const OpBase *opBase);

// creates a new Foreach operation
OpBase *NewForeachOp
(
	const ExecutionPlan *plan  // execution plan
) {
    OpForeach *op = rm_calloc(1, sizeof(OpForeach));

	op->first          = true;
	op->records        = NULL;
	op->supplier       = NULL;
	op->argument_list  = NULL;
	op->first_embedded = NULL;

    OpBase_Init((OpBase *)op, OPType_FOREACH, "Foreach", ForeachInit,
			ForeachConsume, ForeachReset, NULL, ForeachClone, ForeachFree,
			false, plan);

	return (OpBase *)op;
}

static OpResult ForeachInit
(
    OpBase *opBase  // foreach operation to initialize
) {
    OpForeach *op = (OpForeach *)opBase;

	// if number of children is larger than one --> we have a supplier and 
	// a consumer. otherwise, we have only a consumer (using a static list)
	if(op->op.childCount > 1) {
		op->supplier = op->op.children[0];
		op->first_embedded = op->op.children[1];
	} else {
		op->first_embedded = op->op.children[0];
	}

	// search for the ArgumentList op on the embedded sub-execution plan chain
	OpBase *argument_list = op->first_embedded;
	while(argument_list->childCount > 0) {
		argument_list = argument_list->children[0];
	}
	op->argument_list = (ArgumentList *)argument_list;

	// validate found operation type, expecting ArgumentList
	ASSERT(OpBase_Type((const OpBase *)op->argument_list) == OPType_ARGUMENT_LIST);

    return OP_OK;
}

// if there is a record to return, it is returned. Otherwise, returns NULL
static Record _handoff(OpForeach *op) {
	Record r = NULL;
	ASSERT(op->records != NULL);
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

	// if aggregation already occurred, return a record if not depleted
	if(!op->first) {
		return _handoff(op);
	}

	// mark that the aggregation has occurred, so it won't occur again
	op->first = false;

	// construct an array of records to hold all consumed records (eagerly)
	op->records = array_new(Record, 1);

	Record r = NULL;
	if(op->supplier) {
		while((r = OpBase_Consume(op->supplier))) {
			// persist scalars from previous ops, which may be freed before the
			// records are handed of
			// Record_PersistScalars(r);
			array_append(op->records, r);
		}
	} else {
		// static list, create a dummy empty record just to kick start the
		// argument-list operation
		r = OpBase_CreateRecord((OpBase *)op);
		array_append(op->records, r);

		op->first = false;
	}

	// plant a clone of the list of arguments in argument_list operation
	uint nrecords = array_len(op->records);
	Record *clone_list = array_new(Record, nrecords);
	array_clone_with_cb(clone_list, op->records, OpBase_CloneRecord);
	ArgumentList_AddRecordList(op->argument_list, clone_list);

	// call consume on first_embedded op. The result is thrown away.
	while((r = OpBase_Consume(op->first_embedded))) {
		OpBase_DeleteRecord(r);
	};

	return _handoff(op);
}

static OpResult ForeachReset
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;
	op->first = true;

	// free records still held by this operation
	if(op->records) {
		// free record list components
		uint nrecords = array_len(op->records);
		for(uint i = 0; i < nrecords; i++) {
			OpBase_DeleteRecord(op->records[i]);
		}

		array_free(op->records);
	}
	op->records = NULL;

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
		// free record list components
		uint nrecords = array_len(_op->records);
		for(uint i = 0; i < nrecords; i++) {
			OpBase_DeleteRecord(_op->records[i]);
		}

		array_free(_op->records);
	}
	_op->records = NULL;
}
