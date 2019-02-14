/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "aggregate.h"

AggCtx *Agg_Reduce(void *ctx, StepFunc f, ReduceFunc reduce) {
  AggCtx *ac = Agg_NewCtx(ctx);
  ac->Step = f;
  ac->ReduceNext = reduce;
  return ac;
}

AggCtx *Agg_NewCtx(void *fctx) {
    AggCtx *ac = malloc(sizeof(AggCtx));
    ac->err = NULL;
    ac->fctx = fctx;
    ac->result = SI_NullVal();
    ac->Step = NULL;
    ac->ReduceNext = NULL;
    return ac;
}

void AggCtx_Free(AggCtx *ctx) {
  free(ctx->fctx);
  SIValue_Free(&ctx->result);
  free(ctx);
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

inline void *Agg_FuncCtx(AggCtx *ctx) { return ctx->fctx; }

inline void Agg_SetResult(struct AggCtx *ctx, SIValue v) {
    ctx->result = v;
}