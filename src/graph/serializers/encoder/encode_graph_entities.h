/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../graphcontext.h"
#include "../../../redismodule.h"

void RdbSaveNodes(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveDeletedNodes(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveEdges(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveDeletedEdges(RedisModuleIO *rdb, GraphContext *gc);