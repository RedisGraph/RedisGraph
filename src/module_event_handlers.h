/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "redismodule.h"

// Register event handlers for Redis server, keyspace and fork events.
void RegisterEventHandlers(RedisModuleCtx *ctx);
