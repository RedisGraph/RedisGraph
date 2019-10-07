/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../util/heap.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	AR_ExpNode **exps;          // Expression to sort by.
	uint *record_offsets;       // Record IDs corresponding to each sort expression.
	heap_t *heap;               // Holds top n records.
	Record *buffer;             // Holds all records.
	uint limit;                 // Total number of records to produce, 0 no limit.
	int direction;              // Ascending / desending.
} OpSort;

/* Creates a new Sort operation */
OpBase *NewSortOp(const ExecutionPlan *plan, AR_ExpNode **exps, int direction, unsigned int limit);

