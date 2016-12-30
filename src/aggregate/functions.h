#ifndef __AGG_FUNCTIONS_H__
#define __AGG_FUNCTIONS_H__

#include "agg_ctx.h"

typedef AggCtx* (*AggFuncInit)(void);

AggCtx* Agg_SumFunc();
AggCtx* Agg_AvgFunc();
AggCtx* Agg_MaxFunc();
AggCtx* Agg_MinFunc();
AggCtx* Agg_CountFunc();

void Agg_RegisterFuncs();

#endif