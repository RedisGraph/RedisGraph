/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

// OP Unwind
typedef struct {
	OpBase op;
	SIValue list;         // list which the unwind operation is performed on
	AR_ExpNode *exp;      // arithmetic expression (evaluated as an SIArray)
	uint listIdx;         // current list index
	uint listLen;         // length of the list currently being traversed
	int unwindRecIdx;     // update record at this index
	Record currentRecord; // record to clone and add a value extracted from the list
	bool free_rec;        // free the current record when done using or not
} OpUnwind;

// creates a new Unwind operation
OpBase *NewUnwindOp
(
	const ExecutionPlan *plan,
	AR_ExpNode *exp
);

