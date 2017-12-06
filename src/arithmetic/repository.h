#ifndef __AGG_FUNC_REPO_H__
#define __AGG_FUNC_REPO_H__

#include "agg_ctx.h"
#include "agg_funcs.h"

void Agg_GetFunc(const char* name, AggCtx** ctx);
int Agg_RegisterFunc(const char* name, AggFuncInit f);

#endif