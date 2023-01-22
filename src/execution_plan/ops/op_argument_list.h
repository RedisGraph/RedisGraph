/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

// the ArgumentList operation holds an internal Record list that it emits
// one-by-one exactly once

typedef struct {
	OpBase op;              // OpBase
	const Record *records;  // internal Record list
	uint rec_len;           // number of records to emit
	uint rec_idx;           // index of current record
} ArgumentList;

OpBase *NewArgumentListOp
(
	const ExecutionPlan *plan
);

void ArgumentList_AddRecordList
(
	ArgumentList *arg,  // Argument operation to plant the list in
	Record *records     // record list
);

