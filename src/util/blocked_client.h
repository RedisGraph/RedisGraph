/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "RG.h"

// create blocked client and report start time
RedisModuleBlockedClient *RedisGraph_BlockClient
(
    RedisModuleCtx *ctx
);

// unblock blocked client and report end time
void RedisGraph_UnblockClient
(
    RedisModuleBlockedClient *bc
);