/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../arithmetic_expression.h"
#include "../../filter_tree/filter_tree.h"

typedef struct {
	AR_ExpNode *variable;
	FT_FilterNode *ft;
	AR_ExpNode *eval_exp;
} AR_ComprehensionCtx;

void Register_ComprehensionFuncs(void);

