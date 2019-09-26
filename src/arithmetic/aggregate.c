/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "aggregate.h"
#include "../util/rmalloc.h"

AggCtx *Agg_Reduce(void *ctx, StepFunc f, ReduceFunc reduce, InnerData_New innerDataNew,
				   AggCtx_InnerData_Free innerDataFree) {
	AggCtx *ac = Agg_NewCtx(ctx);
	ac->Step = f;
	ac->ReduceNext = reduce;
	ac->InnerData_New = innerDataNew;
	ac->InnerData_Free = innerDataFree;
	return ac;
}

AggCtx *Agg_NewCtx(void *fctx) {
	AggCtx *ac = rm_malloc(sizeof(AggCtx));
	ac->err = NULL;
	ac->fctx = fctx;
	ac->result = SI_NullVal();
	ac->Step = NULL;
	ac->ReduceNext = NULL;
	ac->InnerData_New = NULL;
	ac->InnerData_Free = NULL;
	return ac;
}

AggCtx *Agg_CloneCtx(AggCtx *ctx) {
	void *fctx = ctx->InnerData_New();
	return Agg_Reduce(fctx, ctx->Step, ctx->ReduceNext, ctx->InnerData_New, ctx->InnerData_Free);
}

void AggCtx_Free(AggCtx *ctx) {
	ctx->InnerData_Free(ctx);
	SIValue_Free(&ctx->result);
	rm_free(ctx);
}

int Agg_SetError(AggCtx *ctx, AggError *err) {
	ctx->err = err;
	return AGG_ERR;
}

int Agg_Step(AggCtx *ctx, SIValue *argv, int argc) {
	return ctx->Step(ctx, argv, argc);
}

int Agg_Finalize(AggCtx *ctx) {
	return ctx->ReduceNext(ctx);
}

inline void *Agg_FuncCtx(AggCtx *ctx) {
	return ctx->fctx;
}

inline void Agg_SetResult(struct AggCtx *ctx, SIValue v) {
	ctx->result = v;
}
