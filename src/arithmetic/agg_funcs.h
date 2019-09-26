/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __AGG_FUNCTIONS_H__
#define __AGG_FUNCTIONS_H__

#include "agg_ctx.h"

typedef AggCtx *(*AggFuncInit)(bool);

AggCtx *Agg_SumFunc(bool distinct);
AggCtx *Agg_AvgFunc(bool distinct);
AggCtx *Agg_MaxFunc(bool distinct);
AggCtx *Agg_MinFunc(bool distinct);
AggCtx *Agg_CountFunc(bool distinct);
AggCtx *Agg_PercContFunc(bool distinct);
AggCtx *Agg_PercDiscFunc(bool distinct);
AggCtx *Agg_stDev(bool distinct);
AggCtx *Agg_CollectFunc(bool distinct);

void Agg_RegisterFuncs();

#endif