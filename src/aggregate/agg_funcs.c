#include <float.h>
#include "agg_funcs.h"
#include "aggregate.h"
#include "repository.h"
#include "../value.h"

typedef struct {
    size_t num;
    double total;
} __agg_sumCtx;

int __agg_sumStep(AggCtx *ctx, SIValue *argv, int argc) {
    // convert the value of the input sequence to a double if possible
    double n;
    if (!SIValue_ToDouble(&argv[0], &n)) {
        if (!SIValue_IsNullPtr(&argv[0])) {
            // not convertible to double!
            return Agg_SetError(ctx,
                                "SUM Could not convert upstream value to double");
        } else {
            return AGG_OK;
        }
    }
    
    __agg_sumCtx *ac = Agg_FuncCtx(ctx);

    ac->num++;
    ac->total += n;

    return AGG_OK;
}

int __agg_sumReduceNext(AggCtx *ctx) {
    __agg_sumCtx *ac = Agg_FuncCtx(ctx);
    Agg_SetResult(ctx, SI_DoubleVal(ac->total));
    return AGG_OK;
}

AggCtx* Agg_SumFunc() {
    __agg_sumCtx *ac = malloc(sizeof(__agg_sumCtx));
    ac->num = 0;
    ac->total = 0;
    
    return Agg_Reduce(ac, __agg_sumStep, __agg_sumReduceNext);
}

//------------------------------------------------------------------------

typedef struct {
    size_t count;
    double total;
} __agg_avgCtx;

int __agg_avgStep(AggCtx *ctx, SIValue *argv, int argc) {
    // convert the value of the input sequence to a double if possible
    double n;
    if (!SIValue_ToDouble(&argv[0], &n)) {
        if (!SIValue_IsNullPtr(&argv[0])) {
            // not convertible to double!
            return Agg_SetError(ctx,
                                "AVG Could not convert upstream value to double");
        } else {
            return AGG_OK;
        }
    }
    
    __agg_avgCtx *ac = Agg_FuncCtx(ctx);

    ac->count++;
    ac->total += n;

    return AGG_OK;
}

int __agg_avgReduceNext(AggCtx *ctx) {
    __agg_avgCtx *ac = Agg_FuncCtx(ctx);
    if(ac->count > 0) {
        Agg_SetResult(ctx, SI_DoubleVal(ac->total / ac->count));
    } else {
        Agg_SetResult(ctx, SI_DoubleVal(0));
    }
    return AGG_OK;
}

AggCtx* Agg_AvgFunc() {
    __agg_avgCtx *ac = malloc(sizeof(__agg_avgCtx));
    ac->count = 0;
    ac->total = 0;
    
    return Agg_Reduce(ac, __agg_avgStep, __agg_avgReduceNext);
}

//------------------------------------------------------------------------

typedef struct {
    double max;
} __agg_maxCtx;

int __agg_maxStep(AggCtx *ctx, SIValue *argv, int argc) {
    // convert the value of the input sequence to a double if possible
    double n;
    if (!SIValue_ToDouble(&argv[0], &n)) {
        if (!SIValue_IsNullPtr(&argv[0])) {
            // not convertible to double!
            return Agg_SetError(ctx,
                                "MAX Could not convert upstream value to double");
        } else {
            return AGG_OK;
        }
    }
    
    __agg_maxCtx *ac = Agg_FuncCtx(ctx);
    if(n > ac->max) {
        ac->max = n;
    }

    return AGG_OK;
}

int __agg_maxReduceNext(AggCtx *ctx) {
    __agg_maxCtx *ac = Agg_FuncCtx(ctx);
    Agg_SetResult(ctx, SI_DoubleVal(ac->max));
    return AGG_OK;
}

AggCtx* Agg_MaxFunc() {
    __agg_maxCtx *ac = malloc(sizeof(__agg_maxCtx));
    ac->max = -DBL_MAX;
    
    return Agg_Reduce(ac, __agg_maxStep, __agg_maxReduceNext);
}

//------------------------------------------------------------------------

typedef struct {
    double min;
} __agg_minCtx;

int __agg_minStep(AggCtx *ctx, SIValue *argv, int argc) {
    // convert the value of the input sequence to a double if possible
    double n;
    if (!SIValue_ToDouble(&argv[0], &n)) {
        if (!SIValue_IsNullPtr(&argv[0])) {
            // not convertible to double!
            return Agg_SetError(ctx,
                                "MIN Could not convert upstream value to double");
        } else {
            return AGG_OK;
        }
    }
    
    __agg_minCtx *ac = Agg_FuncCtx(ctx);
    if(n < ac->min) {
        ac->min = n;
    }

    return AGG_OK;
}

int __agg_minReduceNext(AggCtx *ctx) {
    __agg_minCtx *ac = Agg_FuncCtx(ctx);
    Agg_SetResult(ctx, SI_DoubleVal(ac->min));
    return AGG_OK;
}

AggCtx* Agg_MinFunc() {
    __agg_minCtx *ac = malloc(sizeof(__agg_minCtx));
    ac->min = DBL_MAX;
    
    return Agg_Reduce(ac, __agg_minStep, __agg_minReduceNext);
}

//------------------------------------------------------------------------

typedef struct {
    size_t count;
} __agg_countCtx;

int __agg_countStep(AggCtx *ctx, SIValue *argv, int argc) {
    __agg_avgCtx *ac = Agg_FuncCtx(ctx);
    ac->count++;
    return AGG_OK;
}

int __agg_countReduceNext(AggCtx *ctx) {
    __agg_countCtx *ac = Agg_FuncCtx(ctx);
    Agg_SetResult(ctx, SI_DoubleVal(ac->count));
    return AGG_OK;
}

AggCtx* Agg_CountFunc() {
    __agg_countCtx *ac = malloc(sizeof(__agg_countCtx));
    ac->count = 0;
    
    return Agg_Reduce(ac, __agg_countStep, __agg_countReduceNext);
}

//------------------------------------------------------------------------

void Agg_RegisterFuncs() {
    Agg_RegisterFunc("sum", Agg_SumFunc);
    Agg_RegisterFunc("avg", Agg_AvgFunc);
    Agg_RegisterFunc("max", Agg_MaxFunc);
    Agg_RegisterFunc("min", Agg_MinFunc);
    Agg_RegisterFunc("count", Agg_CountFunc);
}