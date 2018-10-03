/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdlib.h>
#include <string.h>
#include "../rmutil/util.h"
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
    context->argc = argc;
    context->argv = RMUtil_RetainArgv(ctx, argv, argc);

    return context;
}

void BulkInsertContext_Free
(
    BulkInsertContext* ctx
) {
    RMUtil_FreeRetainArgv(ctx->argv, ctx->argc);
    free(ctx);
}
