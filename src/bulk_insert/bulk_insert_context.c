/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdlib.h>
#include <string.h>
#include "./bulk_insert_context.h"

BulkInsertContext* BulkInsertContext_New
(
    RedisModuleCtx *ctx,
    RedisModuleBlockedClient *bc,
    RedisModuleString **argv,
    int argc
) {
    BulkInsertContext *context = malloc(sizeof(BulkInsertContext));

    context->bc = bc;
    context->argv = argv;
    context->argc = argc;
    
    // Retain strings arguments.
    for(int i = 0; i < argc; i++)
        RedisModule_RetainString(ctx, argv[i]);

    context->argv = malloc(sizeof(RedisModuleString*) * argc);
    memcpy(context->argv, argv, argc * sizeof(RedisModuleString*));

    return context;
}

void BulkInsertContext_Free
(
    BulkInsertContext* ctx
) {
    for(int i = 0; i < ctx->argc; i++)
        RedisModule_Free(ctx->argv[i]);
    free(ctx->argv);
    free(ctx);
}
