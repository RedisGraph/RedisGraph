/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../../../util/heap.h"
#include "../runtime_execution_plan.h"
#include "../../../../arithmetic/arithmetic_expression.h"

typedef struct {
	RT_OpBase op;
	uint *record_offsets;       // All Record offsets containing values to sort by
	heap_t *heap;               // Holds top n records
	Record *buffer;             // Holds all records
	uint skip;                  // Total number of records to skip
	uint limit;                 // Total number of records to produce
	int *directions;            // Array of sort directions(ascending / desending) for each item
	AR_ExpNode **exps;          // Projected expressons
} RT_OpSort;

// Creates a new Sort operation
RT_OpBase *RT_NewSortOp(const RT_ExecutionPlan *plan, AR_ExpNode **exps, int *directions);
