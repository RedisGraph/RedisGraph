/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef SERIALIZE_GRAPH_H
#define SERIALIZE_GRAPH_H

#include "../../redismodule.h"
#include "../../schema/schema.h"
#include "../graphcontext.h"

void RdbLoadGraph(RedisModuleIO *rdb, GraphContext *gc);
void RdbSaveGraph(RedisModuleIO *rdb, GraphContext *gc);

#endif
