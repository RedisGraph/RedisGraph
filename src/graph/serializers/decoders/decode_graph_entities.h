/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../graphcontext.h"
#include "../../../redismodule.h"

void RdbLoadNodes(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadDeletedNodes(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadEdges(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadDeletedEdges(RedisModuleIO *rdb, GraphContext *gc);