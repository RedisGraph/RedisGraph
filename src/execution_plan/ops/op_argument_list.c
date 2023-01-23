/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op.h"
#include "op_argument_list.h"

// forward declarations
static Record ArgumentListConsume(OpBase *opBase);
static OpResult ArgumentListReset(OpBase *opBase);
static OpBase *ArgumentListClone(const ExecutionPlan *plan, const OpBase *opBase);

OpBase *NewArgumentListOp
(
	const ExecutionPlan *plan
) {
	ArgumentList *op = rm_malloc(sizeof(ArgumentList));
	op->rec_len = 0;
	op->rec_idx = 0;
	op->records = NULL;

	// set our Op operations
	OpBase_Init((OpBase *)op, OPType_ARGUMENT_LIST, "Argument List", NULL,
			ArgumentListConsume, ArgumentListReset, NULL, ArgumentListClone,
			NULL, false, plan);

	return (OpBase *)op;
}

void ArgumentList_AddRecordList
(
	ArgumentList *op,
	Record *records
) {
	ASSERT(op->records == NULL && "insert into a populated ArgumentList");
	op->rec_idx = 0;
	op->records = records;
	op->rec_len = array_len(records);
}

static Record ArgumentListConsume
(
	OpBase *opBase
) {
	ArgumentList *op = (ArgumentList *)opBase;

	ASSERT(op->records != NULL);

	if(op->rec_idx < op->rec_len) {
		return op->records[op->rec_idx++];
	}

	// depleted!
    return NULL;
}

static OpResult ArgumentListReset
(
	OpBase *opBase
) {
	ArgumentList *op = (ArgumentList *)opBase;

	op->records = NULL;
	op->rec_len = 0;
	op->rec_idx = 0;

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
