/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../value.h"
#include "../../datatypes/set.h"


typedef struct {
	SIValue result;
	void *private_ctx;
} AggregateCtx;

void Register_AggFuncs(void);

SIValue Aggregate_GetResult(AggregateCtx *ctx);

