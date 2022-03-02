/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../value.h"
#include "../../datatypes/set.h"

typedef SIValue AggregateResult;
AggregateResult AGGREGATE_OK;

typedef struct {
	SIValue result;
	void *private_ctx;
} AggregateCtx;

void Register_AggFuncs(void);

SIValue Aggregate_GetResult
(
	AggregateCtx *ctx
);

// finalize the result of an aggregate function
void Aggregate_SetResult(
	AggregateCtx *ctx,
	SIValue result
);

void Aggregate_Free
(
	AggregateCtx *ctx
);

//------------------------------------------------------------------------------
// Aggregation default value generators
//------------------------------------------------------------------------------

// default double aggregation value
SIValue Default_Double();

// default long aggregation value
SIValue Default_Long();

// default array aggregation value
SIValue Default_Array();

