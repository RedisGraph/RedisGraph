/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"

int MGraph_Slowlog(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);
