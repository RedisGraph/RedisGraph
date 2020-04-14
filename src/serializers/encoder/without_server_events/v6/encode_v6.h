/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../../serializers_include.h"

void RdbSaveGraphContext_v6(RedisModuleIO *rdb, void *value);
void RdbSaveGraph_v6(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveSchema_v6(RedisModuleIO *rdb, Schema *s);
