/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../../serializers_include.h"

GraphContext *RdbLoadGraphContext_v4(RedisModuleIO *rdb);
void RdbLoadGraph_v4(RedisModuleIO *rdb, GraphContext *gc);
Index *RdbLoadIndex_v4(RedisModuleIO *rdb, GraphContext *gc);
Schema *RdbLoadSchema_v4(RedisModuleIO *rdb);
