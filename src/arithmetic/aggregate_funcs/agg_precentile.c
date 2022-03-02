/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"

//------------------------------------------------------------------------------
// Precentile
//------------------------------------------------------------------------------

typedef struct {
	double percentile;
	double *values;
} _agg_PercCtx;

// This function is agnostic as to percentile method
AggregateResult AGG_PERC(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[2].ptrval;
	_agg_PercCtx *perc_ctx = ctx->private_ctx;

	// On the first invocation, initialize the context.
	if(ctx->private_ctx == NULL) {
		ctx->private_ctx = rm_calloc(1, sizeof(_agg_PercCtx));
		perc_ctx = ctx->private_ctx;
		// The second argument is the requested percentile, which we only
		// need to apply on the first function invocation.
		SIValue_ToDouble(&argv[1], &perc_ctx->percentile);
		perc_ctx->values = array_new(double, 1024);
		if(perc_ctx->percentile < 0 || perc_ctx->percentile > 1) {
			ErrorCtx_SetError("Invalid input - '%f' is not a valid argument, must be a number in the range 0.0 to 1.0",
							  perc_ctx->percentile);
		}
	}

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	double n;
	SIValue_ToDouble(&v, &n);
	array_append(perc_ctx->values, n);

	return AGGREGATE_OK;
}

void PercDiscFinalize(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	_agg_PercCtx *perc_ctx = ctx->private_ctx;
	if(perc_ctx == NULL) return;

	uint count = array_len(perc_ctx->values);
	if(count == 0) {
		Aggregate_SetResult(ctx, SI_NullVal());
	} else {
		QSORT(double, perc_ctx->values, count, ISLT);

		// If perc_ctx->percentile == 0, employing this formula would give an index of -1
		int idx = perc_ctx->percentile > 0 ? ceil(perc_ctx->percentile * count) - 1 : 0;
		double n = perc_ctx->values[idx];
		Aggregate_SetResult(ctx, SI_DoubleVal(n));
	}
}

void PercContFinalize(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	_agg_PercCtx *perc_ctx = ctx->private_ctx;
	if(perc_ctx == NULL) return;

	uint count = array_len(perc_ctx->values);
	if(count == 0) {
		Aggregate_SetResult(ctx, SI_NullVal());
	} else {
		QSORT(double, perc_ctx->values, count, ISLT);

		if(perc_ctx->percentile == 1.0 || count == 1) {
			Aggregate_SetResult(ctx, SI_DoubleVal(perc_ctx->values[count - 1]));
		}

		double int_val, fraction_val;
		double float_idx = perc_ctx->percentile * (count - 1);
		// Split the temp value into its integer and fractional values
		fraction_val = modf(float_idx, &int_val);
		int index = int_val; // Casting the integral part of the value to an int for convenience

		if(!fraction_val) {
			// A valid index was requested, so we can directly return a value
			Aggregate_SetResult(ctx, SI_DoubleVal(perc_ctx->values[index]));
			return;
		}

		double lhs, rhs;
		lhs = perc_ctx->values[index] * (1 - fraction_val);
		rhs = perc_ctx->values[index + 1] * fraction_val;

		Aggregate_SetResult(ctx, SI_DoubleVal(lhs + rhs));
	}
}

void Percentile_Free(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	SIValue_Free(ctx->result);
	if(ctx->private_ctx) {
		_agg_PercCtx *perc_ctx = ctx->private_ctx;
		array_free(perc_ctx->values);
		rm_free(ctx->private_ctx);
	}
	rm_free(ctx);
}

Register_PRECENTILE(void) {
	SIType *types;
	AR_FuncDesc *func_desc;

	types = array_new(SIType, 3);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("percentileDisc", AGG_PERC, 3, 3, types,
			Percentile_Free, PercDiscFinalize, SI_NullVal);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("percentileCont", AGG_PERC, 3, 3, types,
			Percentile_Free, PercContFinalize, SI_NullVal);
	AR_RegFunc(func_desc);
}

