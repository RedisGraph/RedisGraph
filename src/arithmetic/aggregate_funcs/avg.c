/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

//------------------------------------------------------------------------------
// Avg
//------------------------------------------------------------------------------

// avarage context
typedef struct {
	size_t count;    // number of elements summed
	double total;    // sum of all elements
	bool overflow;   // track numeric overflow
	SIValue result;  // computed average
} AvgCtx;

// return true if adding a and b will overflow
// values have the same MSB, adding will enlarge the total
#define ABOUT_TO_OVERFLOW(a, b) (signbit((a)) == signbit((b)) && \
	   (fabs((a)) > (DBL_MAX - fabs((b))))) 

// avarage "step" function expects 2 arguments:
// 1. aggregation context
// 2. value to aggregate
AggregateResult AGG_AVG
(
	SIValue *argv,
	int argc
) {
	SIValue val = argv[0];
	AvgCtx *ctx = argv[1].ptrval;

	// check input
	if(SI_TYPE(val) == T_NULL) return AGGREGATE_OK;

	long double v = SI_GET_NUMERIC(val);

	avg_ctx->count ++; // increment count

	//--------------------------------------------------------------------------
	// check for overflow
	//--------------------------------------------------------------------------

	// if we've already overflowed or adding the current value
	// will cause us to overflow, use the incremental averaging algorithm
	if(avg_ctx->overflow || ABOUT_TO_OVERFLOW(avg_ctx->total, v)) {
		// divide the total by the new count
		long double total = avg_ctx->total /= (long double) avg_ctx->count;
		// if this is not the first call using the incremental algorithm,
		// multiply the total by the previous count
		if(avg_ctx->overflow) total *= (long double)(avg_ctx->count - 1);
		// add v/count to total
		total += (v / (long double)avg_ctx->count);
		avg_ctx->total = total;
		avg_ctx->overflow = true;
	} else {
		// no overflow
		avg_ctx->total += v;
	}

	return AGGREGATE_OK;
}

// finalize aggregation
void AvgFinalize
(
	void *ctx_ptr
) {
	ASSERT(ctx_ptr != NULL);

	AvgCtx *ctx = (AvgCtx*)ctx_ptr;
	if(avg_ctx->count > 0) {
		if(avg_ctx->overflow) {
			// used incremental algorithm due to overflow, 'total' is the average
			Aggregate_SetResult(ctx, SI_DoubleVal(ctx->total));
		} else {
			// used traditional algorithm, divide total by count
			Aggregate_SetResult(ctx, SI_DoubleVal(ctx->total / ctx->count));
		}
	}
}

