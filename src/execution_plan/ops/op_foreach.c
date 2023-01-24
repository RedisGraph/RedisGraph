/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_foreach.h"

// forward declarations
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

	op->body           = NULL;
	op->first          = true;
	op->records        = NULL;
	op->supplier       = NULL;
	op->argument_list  = NULL;

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
		op->body = op->op.children[1];
	} else {
		op->body = op->op.children[0];
	}

	// search for the ArgumentList op on the embedded sub-execution plan chain
	OpBase *argument_list = op->body;
	while(argument_list->childCount > 0) {
		argument_list = argument_list->children[0];
	}
	op->argument_list = (ArgumentList *)argument_list;

	// validate found operation type, expecting ArgumentList
	ASSERT(OpBase_Type((const OpBase *)op->argument_list) == OPType_ARGUMENT_LIST);

    return OP_OK;
}

// if there is a record to return, it is returned
// otherwise, returns NULL
static Record _handoff(OpForeach *op) {
	ASSERT(op->records != NULL);

	Record r = NULL;
	if(array_len(op->records)) {
		r = array_pop(op->records);
	}

	return r;
}

// the Foreach consume function aggregates all the records from the supplier if
// it exists, or a dummy record if not, in a list.
// this list is passed AS IS (i.e., no cloning) to the ArgumentList
// operation which passes them ONE-BY-ONE to the Unwind operation which clones
// every record it receives BEFORE performing its modifications (this is what
// allows us to not clone the records in Foreach or ArgumentList!).
// Unwind will not free the records if ArgumentList is its child (i.e., part of
// an execution-plan embedded in Foreach), such that Foreach is responsible for
// the records.
// please notice these notes when modifying Foreach/ArgumentList/Unwind
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
		// eagerly drain supplier
		while((r = OpBase_Consume(op->supplier))) {
			array_append(op->records, r);
		}
	} else {
		// static list, create a dummy empty record just to kick start the
		// argument-list operation
		r = OpBase_CreateRecord((OpBase *)op);
		array_append(op->records, r);
	}

	// plant a clone of the list of arguments in argument_list operation
	ArgumentList_AddRecordList(op->argument_list, op->records);

	// call consume on loop body first op
	// the result is thrown away
	while((r = OpBase_Consume(op->body))) {
		OpBase_DeleteRecord(r);
	}

	return _handoff(op);
}

// free foreach internal data structures
static void _freeInternals
(
	OpForeach *op  // operation to free
) {
	// free records still held by this operation
	if(op->records != NULL) {
		// free record list components
		uint nrecords = array_len(op->records);
		for(uint i = 0; i < nrecords; i++) {
			OpBase_DeleteRecord(op->records[i]);
		}

		array_free(op->records);
		op->records = NULL;
	}
}

static OpResult ForeachReset
(
    OpBase *opBase  // operation
) {
    OpForeach *op = (OpForeach *)opBase;

	op->first = true;

	_freeInternals(op);

	return OP_OK;
}

static OpBase *ForeachClone
(
    const ExecutionPlan *plan,  // plan
    const OpBase *opBase        // operation to clone
) {
    ASSERT(opBase->type == OPType_FOREACH);
	return NewForeachOp(plan);
}

static void ForeachFree
(
	OpBase *op
) {
	OpForeach *_op = (OpForeach *) op;

	_freeInternals(_op);
}
