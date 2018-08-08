/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "repository.h"
#include "../rmutil/vector.h"

typedef struct {
    const char *name;
    AggFuncInit func;
} __aggFuncEntry;

static Vector *__aggRegisteredFuncs = NULL;

static void __agg_initRegistry() {
    if (__aggRegisteredFuncs == NULL) {
        __aggRegisteredFuncs = NewVector(__aggFuncEntry *, 8);
    }
}

int Agg_RegisterFunc(const char* name, AggFuncInit f) {
    __agg_initRegistry();
    __aggFuncEntry *e = malloc(sizeof(__aggFuncEntry));
    e->name = strdup(name);
    e->func = f;
    return Vector_Push(__aggRegisteredFuncs, e);
}

bool Agg_FuncExists(const char* name) {
    if (!__aggRegisteredFuncs) return false;

    for (int i = 0; i < Vector_Size(__aggRegisteredFuncs); i++) {
        __aggFuncEntry *e = NULL;
        Vector_Get(__aggRegisteredFuncs, i, &e);
        if (e != NULL && !strcasecmp(name, e->name)) {
            return true;
        }
    }

    return false;
}

void Agg_GetFunc(const char* name, AggCtx** ctx) {
     if (!__aggRegisteredFuncs) {
         *ctx = NULL;
         return;
     }

    for (int i = 0; i < Vector_Size(__aggRegisteredFuncs); i++) {
        __aggFuncEntry *e = NULL;
        Vector_Get(__aggRegisteredFuncs, i, &e);
        if (e != NULL && !strcasecmp(name, e->name)) {
            *ctx = e->func();
            return;
        }
    }
    *ctx = NULL;
}