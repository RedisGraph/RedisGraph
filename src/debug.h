/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "redismodule.h"

// Register sigsegv signal hander
void setupCrashHandlers(RedisModuleCtx *ctx);

