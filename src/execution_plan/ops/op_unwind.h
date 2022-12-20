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
	OpBase op;                    // must be the first field
	union {
		struct {
			SIValue list;         // list which the unwind operation is performed on
			uint listIdx;         // current list index
		};
		struct {
			int64_t from;         // beginning of range
			int64_t to;           // end of range
			int64_t step;         // step size
			int64_t current;      // current value in range
		};
	};
	bool is_range;        // op is using range otherwise list
	AR_ExpNode *exp;      // arithmetic expression (evaluated as an SIArray)
	int unwindRecIdx;     // update record at this index
	Record currentRecord; // record to clone and add a value extracted from the list
} OpUnwind;

// creates a new Unwind operation
OpBase *NewUnwindOp
(
	const ExecutionPlan *plan,
	AR_ExpNode *exp
);

