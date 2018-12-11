/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GRAPH_QUERY_H
#define GRAPH_QUERY_H

#define REDISMODULE_EXPERIMENTAL_API    // Required for block client.
#include "../redismodule.h"
#include "../parser/ast.h"
#include "../util/thpool/thpool.h"

extern threadpool _thpool;

/* Query context, used for concurent query processing. */
typedef struct {
    RedisModuleBlockedClient *bc;   // Blocked client.
    AST* ast;                 // Parsed AST.
    RedisModuleString *graphName;   // Graph ID.
    double tic[2];                  // timings.
} QueryContext;

int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif
