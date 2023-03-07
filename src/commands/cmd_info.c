/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */
 /* This file contains the implementation of the GRAPH.INFO command. */

#include "RG.h"
#include "redismodule.h"

// GRAPH.INFO key
// GRAPH.INFO key RESET
// GRAPH.INFO key STATS
// GRAPH.INFO key QUERIES
// Dispatch the subcommand.
int Graph_Info
(
    RedisModuleCtx *ctx,
    RedisModuleString **argv,
    const int argc
) {
    ASSERT(ctx);

    if (argc < 2) {
        return RedisModule_WrongArity(ctx);
    }

    RedisModule_ReplyWithError(ctx, "Unimplemented.");

    return REDISMODULE_ERR;
}
