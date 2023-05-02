/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"

typedef void (*FreePrivDataFunc)(RedisModuleCtx*,void*);

// create blocked client and report start time
RedisModuleBlockedClient *RedisGraph_BlockClient
(
    RedisModuleCtx *ctx,
    FreePrivDataFunc free_privdata
);

// unblock blocked client and report end time
void RedisGraph_UnblockClient
(
    RedisModuleBlockedClient *bc,
    void *privdata
);