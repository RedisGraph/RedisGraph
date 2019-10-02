/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __SI_AGREGATE_H__
#define __SI_AGREGATE_H__

#include <stdlib.h>

#include "agg_ctx.h"
#include "../value.h"

#define AGG_OK 1
#define AGG_SKIP 2
#define AGG_EOF 0
#define AGG_ERR -1

#define AGG_STATE_INIT 0
#define AGG_STATE_DONE 1
#define AGG_STATE_ERR 2
#define AGG_STATE_EOF 3

typedef int (*StepFunc)(AggCtx *ctx, SIValue *argv, int argc);
typedef int (*FinalizeFunc)(AggCtx *ctx);
typedef void (*AggCtx_InnerData_Free)(AggCtx *ctx);
typedef void *(*InnerData_New)(AggCtx *ctx);

AggCtx *Agg_NewCtx(StepFunc f, FinalizeFunc finalize, InnerData_New innerDataNew,
				   AggCtx_InnerData_Free innerDataFree, bool isDistinct);
AggCtx *Agg_CloneCtx(AggCtx *ctx);
void AggCtx_Free(AggCtx *ctx);
int Agg_SetError(AggCtx *ctx, AggError *err);
void *Agg_FuncCtx(AggCtx *ctx);
void Agg_SetResult(AggCtx *ctx, SIValue v);

int Agg_Step(AggCtx *ctx, SIValue *argv, int argc);
int Agg_Finalize(AggCtx *ctx);
#endif
