/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "algorithms.h"
#include "../util/arr.h"
#include <stdio.h>
#include <assert.h>

// Array of registered algorithms.
static AlgorithmCtx *algorithms = NULL;

int Algorithms_Register(const char *name, fpAlgorithmRun fp) {
    assert(name && fp);

    if(Algorithms_Get(name) != NULL) {
        // Algorithm already registered.
        return 0;
    }
    
    if(!algorithms) algorithms = array_new(AlgorithmCtx, 1);

    AlgorithmCtx ctx;
    ctx.handler = fp;
    ctx.name = rm_strdup(name);
    algorithms = array_append(algorithms, ctx);

    return 1;
}

fpAlgorithmRun Algorithms_Get(const char *name) {
    assert(name);

    uint algoCount = array_len(algorithms);    
    for(uint i = 0; i < algoCount; i++) {
        if(strcasecmp(algorithms[i].name, name) == 0) {
            return algorithms[i].handler;
        }
    }

    return NULL;
}
