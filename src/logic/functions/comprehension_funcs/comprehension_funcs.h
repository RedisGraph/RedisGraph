/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../../IR/arithmetic_expression/arithmetic_expression.h"
#include "../../../IR/filter_tree/filter_tree.h"

typedef struct {
	const char *variable_str;  // String name of the comprehension's local variable
	int variable_idx;          // Record index of the comprehension's local variable
	FT_FilterNode *ft;         // [optional] The predicate tree to evaluate each element against.
	AR_ExpNode *eval_exp;      // [optional] The projection routine to build each return element.
	Record local_record;       // Record to populate with the input record's values and the list element.
} ListComprehensionCtx;

void Register_ComprehensionFuncs(void);

