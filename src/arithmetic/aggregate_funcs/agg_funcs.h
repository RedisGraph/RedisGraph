/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../value.h"
#include "../func_desc.h"
#include "../../datatypes/set.h"

typedef SIValue AggregateResult;
AggregateResult AGGREGATE_OK;

// create a new aggregation function descriptor
AR_FuncDesc *AR_AggFuncDescNew
(
	const char *name,                   // function name
	AR_Func func,                       // pointer to function
	uint min_argc,                      // minimum number of arguments
	uint max_argc,                      // maximum number of arguments
	SIType *types,                      // acceptable types
	AR_Func_Free free,                  // free aggregation callback
	AR_Func_Finalize finalize,          // finalize aggregation callback
	AR_Func_PrivateData private_data    // generate private data
);

// register all aggregation funcitons
void Register_AggFuncs(void);

// get computed aggregated value
SIValue Aggregate_GetResult
(
	AggregateCtx *ctx
);

// finalize the result of an aggregate function
void Aggregate_SetResult
(
	AggregateCtx *ctx,
	SIValue result
);

void Aggregate_Finalize
(
	AR_FuncDesc *func_desc,
	AggregateCtx *ctx
);

// free aggregation context
void Aggregate_Free
(
	AR_FuncDesc *agg_func,
	AggregateCtx *ctx
);

