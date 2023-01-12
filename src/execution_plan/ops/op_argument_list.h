/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* The Argument operation holds an internal Record that it will emit exactly once. */
typedef struct {
	OpBase op;
	Record *record_list;
} ArgumentList;

OpBase *NewArgumentListOp
(
	const ExecutionPlan *plan,  // plan to bind the operation to
	const char **variables     // bound variables
);

void ArgumentList_AddRecordList
(
	ArgumentList *arg,       // Argument operation to plant the list in
	Record *record_list  // record list
);
