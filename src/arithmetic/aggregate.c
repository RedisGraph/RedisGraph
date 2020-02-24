/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "aggregate.h"
#include "../util/rmalloc.h"

AggCtx *Agg_NewCtx(StepFunc step, FinalizeFunc finalize, AggCtx_PrivateData_New privateDataNew,
				   AggCtx_PrivateData_Free privateDataFree, bool isDistinct) {
	// Allocate.
	AggCtx *ac = rm_malloc(sizeof(AggCtx));
	// Set methods.
	ac->Step = step;
	ac->Finalize = finalize;
	ac->AggCtx_PrivateData_New = privateDataNew;
	ac->AggCtx_PrivateData_Free = privateDataFree;
	// Initialize members.
	ac->isDistinct = isDistinct;
	// This member initialization depends on isDistinct.
	ac->fctx = ac->AggCtx_PrivateData_New(ac);
	ac->err = NULL;
	ac->result = SI_NullVal();
	return ac;
}

AggCtx *Agg_CloneCtx(AggCtx *ctx) {
	return Agg_NewCtx(ctx->Step, ctx->Finalize, ctx->AggCtx_PrivateData_New,
					  ctx->AggCtx_PrivateData_Free, ctx->isDistinct);
}

void AggCtx_Free(AggCtx *ctx) {
	ctx->AggCtx_PrivateData_Free(ctx);
	SIValue_Free(ctx->result);
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
	return ctx->Finalize(ctx);
}

inline void *Agg_FuncCtx(AggCtx *ctx) {
	return ctx->fctx;
}

inline void Agg_SetResult(struct AggCtx *ctx, SIValue v) {
	ctx->result = v;
}

