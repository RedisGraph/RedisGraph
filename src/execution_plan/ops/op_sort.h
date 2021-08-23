/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
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
	uint skip;                  // Total number of records to skip
	uint limit;                 // Total number of records to produce
	int *directions;            // Array of sort directions(ascending / desending) for each item.
	AR_ExpNode **exps;          // Projected expressons.
} OpSort;

/* Creates a new Sort operation */
OpBase *NewSortOp(const ExecutionPlan *plan, AR_ExpNode **exps, int *directions);

