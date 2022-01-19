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
	ctx_clone->result = orig_ctx->result;
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
	// On the first invocation, initialize the context's value.
	if(SI_TYPE(ctx->result) == T_NULL) ctx->result = SI_DoubleVal(0);

	SIValue v = argv[0];
	if(SI_TYPE(v) == T_NULL) return AGGREGATE_OK;

	// Update the total.
	if(SI_TYPE(v) != T_NULL) ctx->result.doubleval += SI_GET_NUMERIC(v);

	return AGGREGATE_OK;
}

//------------------------------------------------------------------------------
// Avg
//------------------------------------------------------------------------------

typedef struct {
	size_t count;
	double total;
	bool overflow;
} _agg_AvgCtx;

AggregateResult AGG_AVG(SIValue *argv, int argc) {
	AggregateCtx *ctx = argv[1].ptrval;
	// on the first invocation, initialize the context
	if(ctx->private_ctx == NULL) ctx->private_ctx = rm_calloc(1, sizeof(_agg_AvgCtx));

	SIValue si_val = argv[0];
	if(SI_TYPE(si_val) == T_NULL) return AGGREGATE_OK;
	long double v = SI_GET_NUMERIC(si_val);

	_agg_AvgCtx *avg_ctx = ctx->private_ctx;

	avg_ctx->count ++; // increment the count

	// if we've already overflowed or adding the current value
	// will cause us to overflow, use the incremental averaging algorithm
	if(avg_ctx->overflow || // already reached overflow
				(signbit(avg_ctx->total) == signbit(v) && // values have the same MSB, adding will enlarge the total
				 (fabs(avg_ctx->total) > (DBL_MAX - fabs(v))))) { // about to overflow
		// divide the total by the new count
		long double total = avg_ctx->total /= (long double) avg_ctx->count;
		// if this is not the first call using the incremental algorithm,
		// multiply the total by the previous count
		if(avg_ctx->overflow) total *= (long double)(avg_ctx->count - 1);
		// add v/count to total
		total += (v / (long double)avg_ctx->count);
		avg_ctx->total = total;
		avg_ctx->overflow = true;
	} else { // no overflow
		avg_ctx->total += v;
	}

	return AGGREGATE_OK;
}

void AvgFinalize(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	_agg_AvgCtx *avg_ctx = ctx->private_ctx;
	if(avg_ctx->count > 0) {
		if(avg_ctx->overflow) {
			// used incremental algorithm due to overflow, 'total' is the average
			Aggregate_SetResult(ctx, SI_DoubleVal(avg_ctx->total));
		} else {
			// used traditional algorithm, divide total by count
			Aggregate_SetResult(ctx, SI_DoubleVal(avg_ctx->total / avg_ctx->count));
		}
	} else Aggregate_SetResult(ctx, SI_DoubleVal(0));
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
	// On the first invocation, initialize the context's value.
	if(SI_TYPE(ctx->result) == T_NULL) ctx->result = SI_LongVal(0);

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
		ctx->private_ctx = rm_calloc(1, sizeof(_agg_AvgCtx));
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
		ctx->private_ctx = rm_calloc(1, sizeof(_agg_AvgCtx));
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
	// On the first invocation, initialize the context's value.
	AggregateCtx *ctx = argv[1].ptrval;
	if(SI_TYPE(ctx->result) == T_NULL) ctx->result = SI_Array(1);

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
	func_desc = AR_FuncDescNew("sum", AGG_SUM, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Avg
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("avg", AGG_AVG, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, AvgFinalize);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Max
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("max", AGG_MAX, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Min
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("min", AGG_MIN, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Count
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("count", AGG_COUNT, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Avg
	//--------------------------------------------------------------------------

	types = array_new(SIType, 3);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("percentileDisc", AGG_PERC, 3, 3, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Percentile_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, PercDiscFinalize);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 3);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("percentileCont", AGG_PERC, 3, 3, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Percentile_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, PercContFinalize);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Standard deviation
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("stDev", AGG_STDEV, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, StDev_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, StDevFinalize);
	AR_RegFunc(func_desc);

	types = array_new(SIType, 2);
	array_append(types, T_NULL | T_INT64 | T_DOUBLE);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("stDevP", AGG_STDEV, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, StDev_Free, Aggregate_Clone);
	AR_SetFinalizeRoutine(func_desc, StDevPFinalize);
	AR_RegFunc(func_desc);

	//--------------------------------------------------------------------------
	// Collect
	//--------------------------------------------------------------------------

	types = array_new(SIType, 2);
	array_append(types, SI_ALL);
	array_append(types, T_PTR);
	func_desc = AR_FuncDescNew("collect", AGG_COLLECT, 2, 2, types, false, true);
	AR_SetPrivateDataRoutines(func_desc, Aggregate_Free, Aggregate_Clone);
	AR_RegFunc(func_desc);
}

SIValue Aggregate_GetResult(AggregateCtx *ctx) {
	return SI_TransferOwnership(&ctx->result);
}
