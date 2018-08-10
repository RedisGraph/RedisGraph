/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef BULKINSERT_CONTEXT_H
#define BULKINSERT_CONTEXT_H

#include "../redismodule.h"

/* Multi threaded bulk insert context. */
typedef struct {
    RedisModuleBlockedClient *bc;   // Blocked client.
    double tic[2];                  // timings.
    RedisModuleString **argv;
    int argc;
} BulkInsertContext;

BulkInsertContext* BulkInsertContext_New
(
    RedisModuleCtx *ctx,
    RedisModuleBlockedClient *bc,
    RedisModuleString **argv,
    int argc
);

void BulkInsertContext_Free
(
    BulkInsertContext* ctx
);

#endif