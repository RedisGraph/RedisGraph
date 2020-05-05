/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../../serializers_include.h"

GraphContext *RdbLoadGraph_v7(RedisModuleIO *rdb);
void RdbLoadNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadDeletedNodes_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadEdges_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadDeletedEdges_v7(RedisModuleIO *rdb, GraphContext *gc);
void RdbLoadGraphSchema_v7(RedisModuleIO *rdb, GraphContext *gc);
