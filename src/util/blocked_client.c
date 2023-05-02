/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "blocked_client.h"

// create blocked client and report start time
RedisModuleBlockedClient *RedisGraph_BlockClient
(
    RedisModuleCtx *ctx,
    FreePrivDataFunc free_privdata
) {
    ASSERT(ctx != NULL);

    // TODO: Add an option to add the free_privdata() function and add it to the
    // RedisModule_BlockClient() function call below (last arg).
    // sig: void (*free_privdata)(RedisModuleCtx*,void*)

    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, free_privdata, 0);
    if(RedisModule_BlockedClientMeasureTimeStart) {
        // report block client start time
        RedisModule_BlockedClientMeasureTimeStart(bc);
    }
    return bc;
}

// unblock blocked client and report end time
void RedisGraph_UnblockClient
(
    RedisModuleBlockedClient *bc,
    void *privdata
) {
    ASSERT(bc != NULL);

    if(RedisModule_BlockedClientMeasureTimeEnd) {
        // report block client end time
        RedisModule_BlockedClientMeasureTimeEnd(bc);
    }
    RedisModule_UnblockClient(bc, privdata);
}
