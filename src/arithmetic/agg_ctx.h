#ifndef __AGG_CTX_H__
#define __AGG_CTX_H__

#include "../value.h"

typedef char AggError;

struct AggCtx {
    void *fctx;
    AggError *err;
    SIValue result;
    int (*Step)(struct AggCtx *ctx, SIValue *argv, int argc);
    int (*ReduceNext)(struct AggCtx *ctx);
};
typedef struct AggCtx AggCtx;

#endif