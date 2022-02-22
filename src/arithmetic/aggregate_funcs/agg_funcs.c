/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "agg_funcs.h"
#include "../../value.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/qsort.h"
#include "../../util/rmalloc.h"
#include "../../datatypes/array.h"
#include <math.h>
#include <float.h>

#define ISLT(a,b) ((*a) < (*b))

typedef SIValue AggregateResult;
AggregateResult AGGREGATE_OK;

// finalize aggregation
void Avg_Finalize
(
	void *ctx_ptr
);

// avarage "step" function expects 2 arguments:
// 1. aggregation context
// 2. value to aggregate
AggregateResult AGG_AVG
(
	SIValue *argv,
	int argc
);

// Routine for freeing a generic aggregate function context.
void Aggregate_Free(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	if(ctx == NULL) return;
	SIValue_Free(ctx->result);
	if(ctx->private_ctx) rm_free(ctx->private_ctx);
	rm_free(ctx);
}

// Routine for cloning a generic aggregate function context.
void *Aggregate_Clone(void *orig) {
	AggregateCtx *orig_ctx = orig;
	AggregateCtx *ctx_clone = rm_malloc(sizeof(AggregateCtx));
	ctx_clone->result = SI_CloneValue(orig_ctx->result);
	ctx_clone->private_ctx = NULL;
	return ctx_clone;
}

// Finalize the result of an aggregate function.
static inline void Aggregate_SetResult(AggregateCtx *ctx, SIValue result) {
	ctx->result = result;
}

//------------------------------------------------------------------------------
// Sum
//------------------------------------------------------------------------------

AggregateResult AGG_SUM(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// Update the total.
	if(SI_TYPE(v) != T_NULL) ctx->result.doubleval += SI_GET_NUMERIC(v);

	return AGGREGATE_OK;
}

//------------------------------------------------------------------------------
// Max
//------------------------------------------------------------------------------

AggregateResult AGG_MAX(SIValue *argv, int argc) {
	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;
	AggregateCtx *ctx = argv[1].ptrval;

	// Update the result if the current element is greater.
	int compared_null;
	if((SIValue_Compare(ctx->result, v, &compared_null) < 0) ||
	   (compared_null == COMPARED_NULL)) {
		ctx->result = v;
	}

	return AGGREGATE_OK;
}

//------------------------------------------------------------------------------
// Min
//------------------------------------------------------------------------------

AggregateResult AGG_MIN(SIValue *argv, int argc) {
	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;
	AggregateCtx *ctx = argv[1].ptrval;

	// Update the result if the current element is lesser.
	int compared_null;
	if((SIValue_Compare(ctx->result, v, &compared_null) > 0) ||
	   (compared_null == COMPARED_NULL)) {
		ctx->result = v;
	}

	return AGGREGATE_OK;
}

//------------------------------------------------------------------------------
// Count
//------------------------------------------------------------------------------

AggregateResult AGG_COUNT(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// Increment the result.
	ctx->result.longval++;

	return AGGREGATE_OK;
}

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

//------------------------------------------------------------------------------
// Standard deviation
//------------------------------------------------------------------------------

typedef struct {
	double *values;
	double total;
} _agg_StDevCtx;

AggregateResult AGG_STDEV(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;
	_agg_StDevCtx *stdev_ctx = ctx->private_ctx;

	// On the first invocation, initialize the context.
	if(ctx->private_ctx == NULL) {
		ctx->private_ctx = rm_calloc(1, sizeof(_agg_StDevCtx));
		stdev_ctx = ctx->private_ctx;
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
	_agg_StDevCtx *stdev_ctx = ctx->private_ctx;

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
	if(ctx->private_ctx) {
		_agg_StDevCtx *stdev_ctx = ctx->private_ctx;
		array_free(stdev_ctx->values);
		rm_free(ctx->private_ctx);
	}
	rm_free(ctx);
}

//------------------------------------------------------------------------------
// Collect
//------------------------------------------------------------------------------

AggregateResult AGG_COLLECT(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// SIArray_Append will clone the added value, ensuring it can be
	// safely accessed for the lifetime of the Collect context.
	SIArray_Append(&ctx->result, v);

	return AGGREGATE_OK;
}

//------------------------------------------------------------------------------
// Function registration
//------------------------------------------------------------------------------

void Register_AggFuncs() {
	SIType *types;
	AR_FuncDesc *func_desc;
	AGGREGATE_OK = SI_NullVal();

	//--------------------------------------------------------------------------
	// Sum
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("sum", AGG_SUM, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetDefaultValue(func_desc, SI_DoubleVal(0));
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Avg
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("avg", AGG_AVG, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, Avg_Finalize);
	AR_SetDefaultValue(func_desc, SI_NullVal());
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Max
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("max", AGG_MAX, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetDefaultValue(func_desc, SI_NullVal());
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Min
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("min", AGG_MIN, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetDefaultValue(func_desc, SI_NullVal());
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Count
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("count", AGG_COUNT, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetDefaultValue(func_desc, SI_LongVal(0));
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// precentile
	//--------------------------------------------------------------------------

	types = array_new(SIType, 3);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("percentileDisc", AGG_PERC, 3, 3, types, false);
	AR_SetPrivateDataRoutines(func_desc, Percentile_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, PercDiscFinalize);
	AR_SetDefaultValue(func_desc, SI_NullVal());
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("percentileCont", AGG_PERC, 3, 3, types, false);
	AR_SetPrivateDataRoutines(func_desc, Percentile_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, PercContFinalize);
	AR_SetDefaultValue(func_desc, SI_NullVal());
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Standard deviation
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("stDev", AGG_STDEV, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, StDev_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, StDevFinalize);
	AR_SetDefaultValue(func_desc, SI_DoubleVal(0));
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("stDevP", AGG_STDEV, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, StDev_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, StDevPFinalize);
	AR_SetDefaultValue(func_desc, SI_DoubleVal(0));
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Collect
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_AggFuncDescNew("collect", AGG_COLLECT, 2, 2, types, false);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetDefaultValue(func_desc, SI_Array(0));
	AR_RegFunc(func_desc);
}

SIValue Aggregate_GetResult(AggregateCtx *ctx) {
	return SI_TransferOwnership(&ctx->result);
}

