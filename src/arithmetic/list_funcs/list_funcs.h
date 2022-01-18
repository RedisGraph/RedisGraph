/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../value.h"
#include "../arithmetic_expression.h"

// reduce function context object
typedef struct {
	const char *variable;     // closure varaible name
	const char *accumulator;  // closure accumulator name
	int variable_idx;         // closure variable record index
	int accumulator_idx;      // closure accumulator record index
	AR_ExpNode *exp;          // expression used for reduction
	Record record;            // internal private record
} ListReduceCtx;

void Register_ListFuncs();

