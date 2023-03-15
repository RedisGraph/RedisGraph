/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op.h"
#include "op_argument_list.h"

// forward declarations
static void ArgumentListFree(OpBase *opBase);
static Record ArgumentListConsume(OpBase *opBase);
static OpResult ArgumentListReset(OpBase *opBase);
static OpBase *ArgumentListClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewArgumentListOp
(
	const ExecutionPlan *plan
) {
	ArgumentList *op = rm_malloc(sizeof(ArgumentList));
	op->records = NULL;

	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT_LIST, "Argument List", NULL,
			ArgumentListConsume, ArgumentListReset, NULL, ArgumentListClone,
			ArgumentListFree, false, plan);

	return (OpBase *)op;
}

void ArgumentList_AddRecordList
(
	ArgumentList *op,
	Record *records
) {
	ASSERT(op->records == NULL && "insert into a populated ArgumentList");
	op->records = records;
}

static Record ArgumentListConsume
(
	OpBase *opBase
) {
	ArgumentList *op = (ArgumentList *)opBase;

	ASSERT(op->records != NULL);
	// first value is NULL, thus terminates execution when popped
	return array_pop(op->records);
}

static OpResult ArgumentListReset
(
	OpBase *opBase
) {
	ArgumentList *op = (ArgumentList *)opBase;

	// free remaining records
	if(op->records != NULL) {
		uint nrecords = array_len(op->records);
		for(uint i = 0; i < nrecords; i++) {
			OpBase_DeleteRecord(op->records[i]);
		}

		array_free(op->records);
		op->records = NULL;
	}

	return OP_OK;
}

static inline OpBase *ArgumentListClone
(
	const ExecutionPlan *plan,
	const OpBase *opBase
) {
	ASSERT(opBase->type == OPType_ARGUMENT_LIST);
	return NewArgumentListOp(plan);
}

static void ArgumentListFree
(
	OpBase *opBase
) {
	ArgumentList *op = (ArgumentList *)opBase;

	// free remaining records
	if(op->records != NULL) {
		uint nrecords = array_len(op->records);
		for(uint i = 1; i < nrecords; i++) {
			OpBase_DeleteRecord(op->records[i]);
		}

		array_free(op->records);
		op->records = NULL;
	}
}
