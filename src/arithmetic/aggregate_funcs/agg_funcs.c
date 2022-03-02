/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "agg_funcs.h"
#include "../../value.h"
#include "../../util/rmalloc.h"

// TODO: might be deprecated?
// routine for cloning a generic aggregate function context
void *Aggregate_Clone(void *orig) {
	AggregateCtx *orig_ctx = orig;
	AggregateCtx *ctx_clone = rm_malloc(sizeof(AggregateCtx));
	ctx_clone->result = SI_CloneValue(orig_ctx->result);
	ctx_clone->private_data = NULL;
	return ctx_clone;
}

// finalize the result of an aggregate function
void Aggregate_SetResult
(
	AggregateCtx *ctx,
	SIValue result
) {
	ctx->result = result;
}

void Aggregate_Finalize
(
	AR_FuncDesc *func_desc,
	AggregateCtx *ctx
) {
	ASSERT(func_desc != NULL);
	ASSERT(ctx != NULL);

	if(func_desc->agg_callbacks.finalize) {
		func_desc->agg_callbacks.finalize(ctx);
	}
}

// get aggregated result
SIValue Aggregate_GetResult
(
	AggregateCtx *ctx
) {
	return SI_TransferOwnership(&ctx->result);
}

//------------------------------------------------------------------------------
// Aggregation default value generators
//------------------------------------------------------------------------------

// default double aggregation value
SIValue Default_Double(void) {
	return SI_DoubleVal(0);
}

// default long aggregation value
SIValue Default_Long(void) {
	return SI_LongVal(0);
}

// default array aggregation value
SIValue Default_Array(void) {
	return SI_Array(0);
}

//------------------------------------------------------------------------------
// Aggregation function registration
//------------------------------------------------------------------------------

void Register_AVG        (void);
void Register_SUM        (void);
void Register_MAX        (void);
void Register_MIN        (void);
void Register_STD        (void);
void Register_COUNT      (void);
void Register_COLLECT    (void);
void Register_PRECENTILE (void);

void Register_AggFuncs() {
	Register_AVG();
	Register_SUM();
	Register_MAX();
	Register_MIN();
	Register_STD();
	Register_COUNT();
	Register_COLLECT();
	Register_PRECENTILE();
}

// routine for freeing a generic aggregate function context
void Aggregate_Free
(
	AR_FuncDesc *agg_func,
	AggregateCtx *ctx
) {
	if(ctx == NULL) return;

	SIValue_Free(ctx->result);

	if(ctx->private_data) agg_func->agg_callbacks.free(ctx->private_data);

	rm_free(ctx);
}

