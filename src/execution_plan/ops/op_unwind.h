/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

// OP Unwind
typedef struct {
	OpBase op;
	SIValue list;          // list which the unwind operation is performed on
	AR_ExpNode *exp;       // arithmetic expression (evaluated as an SIArray)
	uint listIdx;          // current list index
	uint batchIdx;         // index into input batch
	RecordBatch in_batch;  // input batch
	int unwindRecIdx;      // update record at this index
} OpUnwind;

/* Creates a new Unwind operation */
OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp);

