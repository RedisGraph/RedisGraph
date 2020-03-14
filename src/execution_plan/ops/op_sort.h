/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
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
	uint *record_offsets;       // All Record offsets containing values to sort by.
	heap_t *heap;               // Holds top n records.
	Record *buffer;             // Holds all records.
	AR_ExpNode *limit_expr;     // Number of emitted records in the scope;
	AR_ExpNode *skip_expr;      // Number of skiped records in the scope;
	uint limit;                 // Total number of records to produce, 0 no limit.
	int *directions;            // Array of sort directions(ascending / desending) for each item.
	AR_ExpNode **exps;          // Projected expressons.
} OpSort;

/* Creates a new Sort operation */
OpBase *NewSortOp(const ExecutionPlan *plan, AR_ExpNode **exps, int *directions,
				  AR_ExpNode *limit_expr, AR_ExpNode *skip_expr);

