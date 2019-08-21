/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef SERIALIZE_GRAPH_H
#define SERIALIZE_GRAPH_H

#include "../../graphcontext.h"
#include "../../../redismodule.h"

void RdbSaveGraph(RedisModuleIO *rdb, GraphContext *gc);

#endif
