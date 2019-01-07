/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "cmd_call.h"
#include "../algorithms/algorithms.h"

#include <stdlib.h>
#include <assert.h>

int MGraph_Call(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc != 2) return RedisModule_WrongArity(ctx);

    // See if algorithem is available.
    const char *algorithm = RedisModule_StringPtrLen(argv[1], NULL);
    
    fpAlgorithmRun handler = Algorithms_Get(algorithm);
    if(!handler) {
        char *error = NULL;
        asprintf(&error, "%s", algorithm);
        RedisModule_ReplyWithError(ctx, error);
        free(error);
    }

    // TODO: Invoke on a dedicated thread!
    // Call algorithem.
    const char *args[argc-2];
    for(int i = 0; i < argc-2; i++) args[i] = RedisModule_StringPtrLen(argv[2+i], NULL);    
    handler(argc-2, args);
    
    return REDISMODULE_OK;
}
