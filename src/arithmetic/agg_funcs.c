/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <float.h>
#include "agg_funcs.h"
#include "aggregate.h"
#include "repository.h"
#include "../value.h"
#include <math.h>
#include "../util/qsort.h"

#define ISLT(a,b) ((*a) < (*b))

typedef struct {
    size_t num;
    double total;
} __agg_sumCtx;

int __agg_sumStep(AggCtx *ctx, SIValue *argv, int argc) {
    // convert the value of the input sequence to a double if possible
    __agg_sumCtx *ac = Agg_FuncCtx(ctx);

    double n;
    for(int i = 0; i < argc; i ++) {
        if (!SIValue_ToDouble(&argv[i], &n)) {
            if (!SIValue_IsNullPtr(&argv[i])) {
                // not convertible to double!
                return Agg_SetError(ctx,
                                    "SUM Could not convert upstream value to double");
            } else {
                return AGG_OK;
            }
        }

        ac->num++;
        ac->total += n;
    }

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
    __agg_avgCtx *ac = Agg_FuncCtx(ctx);

    double n;
    for(int i = 0; i < argc; i ++) {
        if (!SIValue_ToDouble(&argv[i], &n)) {
            if (!SIValue_IsNullPtr(&argv[i])) {
                // not convertible to double!
                return Agg_SetError(ctx,
                                    "AVG Could not convert upstream value to double");
            } else {
                return AGG_OK;
            }
        }


        ac->count++;
        ac->total += n;
    }

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
    __agg_maxCtx *ac = Agg_FuncCtx(ctx);

    double n;
    for(int i = 0; i < argc; i ++) {
        if (!SIValue_ToDouble(&argv[i], &n)) {
            if (!SIValue_IsNullPtr(&argv[i])) {
                // not convertible to double!
                return Agg_SetError(ctx,
                                    "MAX Could not convert upstream value to double");
            } else {
                return AGG_OK;
            }
        }

        if(n > ac->max) {
            ac->max = n;
        }
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
    __agg_minCtx *ac = Agg_FuncCtx(ctx);

    double n;
    for(int i = 0; i < argc; i ++) {
        if (!SIValue_ToDouble(&argv[i], &n)) {
            if (!SIValue_IsNullPtr(&argv[i])) {
                // not convertible to double!
                return Agg_SetError(ctx,
                                    "MIN Could not convert upstream value to double");
            } else {
                return AGG_OK;
            }
        }

        if(n < ac->min) {
            ac->min = n;
        }
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
    __agg_countCtx *ac = Agg_FuncCtx(ctx);
    // Batch size to this function is always one, so
    // we only need to check the first argument
    if (!SIValue_IsNullPtr(argv)) ac->count ++;

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

typedef struct {
    double percentile;
    double *values;
    size_t count;
    size_t values_allocated;
} __agg_percCtx;

// This function is agnostic as to percentile method
int __agg_percStep(AggCtx *ctx, SIValue *argv, int argc) {
    __agg_percCtx *ac = Agg_FuncCtx(ctx);

    // The last argument is the requested percentile, which we only
    // need to apply on the first function invocation (at which time
    // _agg_percCtx->percentile will be -1)
    if (ac->percentile < 0) {
        if (!SIValue_ToDouble(&argv[argc - 1], &ac->percentile)) {
            return Agg_SetError(ctx,
                    "PERC_DISC Could not convert percentile argument to double");
        }
        if (ac->percentile < 0 || ac->percentile > 1) {
            return Agg_SetError(ctx,
                    "PERC_DISC Invalid input for percentile is not a valid argument, must be a number in the range 0.0 to 1.0");
        }
    }

    if (ac->count + argc - 1 > ac->values_allocated) {
        ac->values_allocated *= 2;
        ac->values = realloc(ac->values, sizeof(double) * ac->values_allocated);
    }

    double n;
    for (int i = 0; i < argc - 1; i ++) {
        if (!SIValue_ToDouble(&argv[i], &n)) {
            if (!SIValue_IsNullPtr(&argv[i])) {
                // not convertible to double!
                return Agg_SetError(ctx,
                        "PERC_DISC Could not convert upstream value to double");
            } else {
                return AGG_OK;
            }
        }
        ac->values[ac->count] = n;
        ac->count++;
    }

    return AGG_OK;
}

int __agg_percDiscReduceNext(AggCtx *ctx) {
    __agg_percCtx *ac = Agg_FuncCtx(ctx);

    QSORT(double, ac->values, ac->count, ISLT);

    // If ac->percentile == 0, employing this formula would give an index of -1
    int idx = ac->percentile > 0 ? ceil(ac->percentile * ac->count) - 1 : 0;
    double n = ac->values[idx];
    Agg_SetResult(ctx, SI_DoubleVal(n));

    free(ac->values);
    return AGG_OK;
}

int __agg_percContReduceNext(AggCtx *ctx) {
    __agg_percCtx *ac = Agg_FuncCtx(ctx);

    QSORT(double, ac->values, ac->count, ISLT);

    if (ac->percentile == 1.0 || ac->count == 1) {
        Agg_SetResult(ctx, SI_DoubleVal(ac->values[ac->count - 1]));
        free(ac->values);
        return AGG_OK;
    }

    double int_val, fraction_val;
    double float_idx = ac->percentile * (ac->count - 1);
    // Split the temp value into its integer and fractional values
    fraction_val = modf(float_idx, &int_val);
    int index = int_val; // Casting the integral part of the value to an int for convenience

    if (!fraction_val) {
        // A valid index was requested, so we can directly return a value
        Agg_SetResult(ctx, SI_DoubleVal(ac->values[index]));
        free(ac->values);
        return AGG_OK;
    }

    double lhs, rhs;
    lhs = ac->values[index] * (1 - fraction_val);
    rhs = ac->values[index + 1] * fraction_val;

    Agg_SetResult(ctx, SI_DoubleVal(lhs + rhs));

    free(ac->values);
    return AGG_OK;
}

// The percentile initializers are identical save for the ReduceNext function they specify
AggCtx* Agg_PercDiscFunc() {
    __agg_percCtx *ac = malloc(sizeof(__agg_percCtx));
    ac->count = 0;
    ac->values = malloc(1024 * sizeof(double));
    ac->values_allocated = 1024;
    // Percentile will be updated by the first call to Step
    ac->percentile = -1;
    return Agg_Reduce(ac, __agg_percStep, __agg_percDiscReduceNext);
}

AggCtx* Agg_PercContFunc() {
    __agg_percCtx *ac = malloc(sizeof(__agg_percCtx));
    ac->count = 0;
    ac->values = malloc(1024 * sizeof(double));
    ac->values_allocated = 1024;
    // Percentile will be updated by the first call to Step
    ac->percentile = -1;
    return Agg_Reduce(ac, __agg_percStep, __agg_percContReduceNext);
}

//------------------------------------------------------------------------

typedef struct {
    double *values;
    double total;
    size_t count;
    size_t values_allocated;
    int is_sampled;
} __agg_stdevCtx;

int __agg_StdevStep(AggCtx *ctx, SIValue *argv, int argc) {
    __agg_stdevCtx *ac = Agg_FuncCtx(ctx);

    if (ac->count + argc > ac->values_allocated) {
        ac->values_allocated *= 2;
        ac->values = realloc(ac->values, sizeof(double) * ac->values_allocated);
    }

    double n;
    for (int i = 0; i < argc; i ++) {
        if (!SIValue_ToDouble(&argv[i], &n)) {
            if (!SIValue_IsNullPtr(&argv[i])) {
                // not convertible to double!
                return Agg_SetError(ctx,
                        "STDEV Could not convert upstream value to double");
            } else {
                return AGG_OK;
            }
        }
        ac->values[ac->count] = n;
        ac->total += n;
        ac->count++;
    }

    return AGG_OK;
}

int __agg_StdevReduceNext(AggCtx *ctx) {
    __agg_stdevCtx *ac = Agg_FuncCtx(ctx);

    if (ac->count < 2) {
        Agg_SetResult(ctx, SI_DoubleVal(0));
        free(ac->values);
        return AGG_OK;
    }

    double mean = ac->total / ac->count;
    long double sum = 0;
    for (int i = 0; i < ac->count; i ++) {
        sum += (ac->values[i] - mean) * (ac->values[i] + mean);
    }
    // is_sampled will be equal to 1 in the Stdev case and 0 in the StdevP case
    double variance = sum / (ac->count - ac->is_sampled);
    double stdev = sqrt(variance);

    Agg_SetResult(ctx, SI_DoubleVal(stdev));

    free(ac->values);
    return AGG_OK;
}

AggCtx* Agg_StdevFunc() {
    __agg_stdevCtx *ac = malloc(sizeof(__agg_stdevCtx));
    ac->is_sampled = 1;
    ac->count = 0;
    ac->total = 0;
    ac->values = malloc(1024 * sizeof(double));
    ac->values_allocated = 1024;
    return Agg_Reduce(ac, __agg_StdevStep, __agg_StdevReduceNext);
}

// StdevP is identical to Stdev save for an altered value we can check for with a bool
AggCtx* Agg_StdevPFunc() {
    AggCtx *func = Agg_StdevFunc();
    __agg_stdevCtx *ac = Agg_FuncCtx(func);
    ac->is_sampled = 0;
    return func;
}

//------------------------------------------------------------------------

void Agg_RegisterFuncs() {
    Agg_RegisterFunc("sum", Agg_SumFunc);
    Agg_RegisterFunc("avg", Agg_AvgFunc);
    Agg_RegisterFunc("max", Agg_MaxFunc);
    Agg_RegisterFunc("min", Agg_MinFunc);
    Agg_RegisterFunc("count", Agg_CountFunc);
    Agg_RegisterFunc("percentileDisc", Agg_PercDiscFunc);
    Agg_RegisterFunc("percentileCont", Agg_PercContFunc);
    Agg_RegisterFunc("stDev", Agg_StdevFunc);
    Agg_RegisterFunc("stDevP", Agg_StdevPFunc);
}