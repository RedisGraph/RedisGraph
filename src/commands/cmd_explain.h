/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GRAPH_EXPLAIN_H
#define GRAPH_EXPLAIN_H

#define REDISMODULE_EXPERIMENTAL_API    // Required for block client.
#include "../redismodule.h"
#include "../parser/ast.h"
#include "../util/thpool/thpool.h"

extern threadpool _thpool;

int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif