/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef COMMAND_CONTEXT_H
#define COMMAND_CONTEXT_H

#include "../redismodule.h"
#include "../parser/ast.h"
#include "../parser/newast.h"

/* Query context, used for concurent query processing. */
typedef struct {
    RedisModuleCtx *ctx;            // Redis module context.
    RedisModuleBlockedClient *bc;   // Blocked client.
    AST **ast;                      // Parsed AST.
    NEWAST *new_ast;                // Parsed AST.
    char *graphName;                // Graph ID.
    double tic[2];                  // Timings.
    RedisModuleString **argv;       // Arguments.
    int argc;                       // Argument count.
} CommandCtx;

// Create a new command context.
CommandCtx* CommandCtx_New
(
    RedisModuleCtx *ctx,            // Redis module context.
    RedisModuleBlockedClient *bc,   // Blocked client.
    AST **ast,
    NEWAST *new_ast,                // Parsed AST.
    RedisModuleString *graphName,   // Graph ID.
    RedisModuleString **argv,       // Arguments.
    int argc                        // Argument count.
);

// Get Redis module context
RedisModuleCtx* CommandCtx_GetRedisCtx
(
    CommandCtx *qctx
);

// Acquire Redis global lock.
void CommandCtx_ThreadSafeContextLock
(
    const CommandCtx *qctx
);

// Release Redis global lock.
void CommandCtx_ThreadSafeContextUnlock
(
    const CommandCtx *qctx
);

// Free command context.
void CommandCtx_Free
(
    CommandCtx* qctx
);

#endif
