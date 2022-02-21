/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"
#include "../../util/rmalloc.h"
#include <math.h>
#include <float.h>

typedef SIValue AggregateResult;
AggregateResult AGGREGATE_OK;

//------------------------------------------------------------------------------
// Avg
//------------------------------------------------------------------------------

// avarage context
typedef struct {
	size_t count;    // number of elements summed
	double total;    // sum of all elements
	bool overflow;   // track numeric overflow
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
	AggregateCtx *ctx = argv[1].ptrval;
	AvgCtx *avg_ctx = ctx->private_ctx;

	// On the first invocation, initialize the context.
	if(ctx->private_ctx == NULL) {
		avg_ctx = ctx->private_ctx = rm_calloc(1, sizeof(AvgCtx));
	}

	// check input
	if(SI_TYPE(val) == T_NULL) return AGGREGATE_OK;

	long double v = SI_GET_NUMERIC(val);

	avg_ctx->count++; // increment count

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

// Routine for cloning a avg aggregate function context.
void *Avg_Clone(void *orig) {
	AvgCtx *orig_ctx = orig;
	AvgCtx *ctx_clone = rm_malloc(sizeof(AvgCtx));
	ctx_clone->total  = orig_ctx->total;
	ctx_clone->overflow = orig_ctx->overflow;
	ctx_clone->count = orig_ctx->count;
	return ctx_clone;
}

// finalize aggregation
void Avg_Finalize
(
	void *ctx_ptr
) {
	ASSERT(ctx_ptr != NULL);

	AggregateCtx *ctx = (AggregateCtx*)ctx_ptr;

	AvgCtx *avg_ctx = ctx->private_ctx;
	if(avg_ctx->count > 0) {
		if(avg_ctx->overflow) {
			// used incremental algorithm due to overflow, 'total' is the average
			ctx->result = SI_DoubleVal(avg_ctx->total);
		} else {
			// used traditional algorithm, divide total by count
			ctx->result = SI_DoubleVal(avg_ctx->total / avg_ctx->count);
		}
	}
}

// Routine for freeing a avg aggregate function context.
void Avg_Free(void *ctx_ptr) {
	AggregateCtx *ctx = ctx_ptr;
	SIValue_Free(ctx->result);
	if(ctx->private_ctx) {
		AvgCtx *avg_ctx = ctx->private_ctx;
		// SIValue_Free(avg_ctx->result);
		// rm_free(ctx->private_ctx);
	}
	rm_free(ctx);
}
