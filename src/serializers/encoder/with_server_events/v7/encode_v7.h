/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../../serializers_include.h"

void RdbSaveGraphContext_v7(RedisModuleIO *rdb, void *value);
void RdbSaveNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveDeletedNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveEdges_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveDeletedEdges_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveGraphSchema_v7(RedisModuleIO *rdb, GraphContext *gc);
