/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __AGG_FUNCTIONS_H__
#define __AGG_FUNCTIONS_H__

#include "agg_ctx.h"

typedef AggCtx *(*AggFuncInit)(void);

AggCtx *Agg_SumFunc();
AggCtx *Agg_AvgFunc();
AggCtx *Agg_MaxFunc();
AggCtx *Agg_MinFunc();
AggCtx *Agg_CountFunc();
AggCtx *Agg_CountDistinctFunc();
AggCtx *Agg_PercContFunc();
AggCtx *Agg_PercDiscFunc();
AggCtx *Agg_stDev();
AggCtx *Agg_CollectFunc();
AggCtx *Agg_CollectDistinctFunc();

void Agg_RegisterFuncs();

#endif