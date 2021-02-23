/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPH_DELETE_H
#define GRAPH_DELETE_H

#include "../redismodule.h"
#include "../util/thpool/thpool.h"

int MGraph_Delete(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

#endif
