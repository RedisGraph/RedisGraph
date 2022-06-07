/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "blocked_client.h"

RedisModuleBlockedClient *RedisGraph_BlockClient
(
    RedisModuleCtx *ctx
) {
    ASSERT(ctx != NULL);

    RedisModuleBlockedClient *bc = RedisModule_BlockClient(ctx, NULL, NULL, NULL, 0);
    if(RedisModule_BlockedClientMeasureTimeStart) {
        // report block client start time
        RedisModule_BlockedClientMeasureTimeStart(bc);
    }
    return bc;
}

void RedisGraph_UnblockClient
(
    RedisModuleBlockedClient *bc
) {
    ASSERT(bc != NULL);

    if(RedisModule_BlockedClientMeasureTimeEnd) {
        // report block client end time
        RedisModule_BlockedClientMeasureTimeEnd(bc);
    }
    RedisModule_UnblockClient(bc, NULL);
}