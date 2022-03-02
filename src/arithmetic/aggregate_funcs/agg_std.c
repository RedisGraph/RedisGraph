/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"
#include "../func_desc.h"
#include "../../util/arr.h"
#include <math.h>

//------------------------------------------------------------------------------
// Standard deviation
//------------------------------------------------------------------------------

typedef struct {
	double *values;
	double total;
} _agg_StDevCtx;

AggregateResult AGG_STDEV(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;
	_agg_StDevCtx *stdev_ctx = ctx->private_data;

	// On the first invocation, initialize the context.
	if(ctx->private_data == NULL) {
		ctx->private_data = rm_calloc(1, sizeof(_agg_StDevCtx));
		stdev_ctx = ctx->private_data;
		stdev_ctx->values = array_new(double, 1024);
	}

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	double n;
	SIValue_ToDouble(&v, &n);
	array_append(stdev_ctx->values, n);
	stdev_ctx->total += n;

	return AGGREGATE_OK;
}

void StDevGenericFinalize(AggregateCtx *ctx, int is_sampled) {
	_agg_StDevCtx *stdev_ctx = ctx->private_data;

	if(stdev_ctx == NULL) return;

	uint count = array_len(stdev_ctx->values);
	if(count - is_sampled == 0) {
		Aggregate_SetResult(ctx, SI_DoubleVal(0));
		return;
	}

	double mean = stdev_ctx->total / count;
	long double sum = 0;
	for(int i = 0; i < count; i ++) {
		sum += (long double)(stdev_ctx->values[i] - mean) * (stdev_ctx->values[i] + mean);
	}
	// is_sampled will be equal to 1 in the Stdev case and 0 in the StdevP case
	double variance = sum / (count - is_sampled);
	double stdev = sqrt(variance);

	Aggregate_SetResult(ctx, SI_DoubleVal(stdev));
}

void StDevFinalize(void *ctx_ptr) {
	StDevGenericFinalize(ctx_ptr, 1);
}

void StDevPFinalize(void *ctx_ptr) {
	StDevGenericFinalize(ctx_ptr, 0);
}

void StDev_Free(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	SIValue_Free(ctx->result);
	if(ctx->private_data) {
		_agg_StDevCtx *stdev_ctx = ctx->private_data;
		array_free(stdev_ctx->values);
		rm_free(ctx->private_data);
	}
	rm_free(ctx);
}

void Register_STD(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("stDev", AGG_STDEV, 2, 2, types, StDev_Free,
			StDevFinalize, Default_Double);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("stDevP", AGG_STDEV, 2, 2, types, StDev_Free,
			StDevPFinalize, Default_Double);
	AR_RegFunc(func_desc);
}

