/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPH_QUERY_H
#define GRAPH_QUERY_H

#include "../redismodule.h"
#include "../util/thpool/thpool.h"

extern threadpool _thpool;

int MGraph_Query(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif
