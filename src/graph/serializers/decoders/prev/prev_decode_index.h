/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../../graphcontext.h"
#include "../../../../index/index.h"
#include "../../../../redismodule.h"

Index* PrevRdbLoadIndex(RedisModuleIO *rdb, GraphContext *gc);
