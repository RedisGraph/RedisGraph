/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../value.h"
#include "../../datatypes/set.h"
#include "../arithmetic_expression.h"


typedef struct {
	SIValue result;
	void *private_ctx;
} AggregateCtx;

void Register_AggFuncs(void);

// check to see if the function operates on distinct results
bool Aggregate_PerformsDistinct(AR_ExpNode *exp);

SIValue Aggregate_GetResult(AggregateCtx *ctx);

