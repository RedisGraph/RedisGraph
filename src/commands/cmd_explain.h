/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPH_EXPLAIN_H
#define GRAPH_EXPLAIN_H

#include "../redismodule.h"
#include "../util/thpool/thpool.h"

extern threadpool _thpool;

int MGraph_Explain(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif
