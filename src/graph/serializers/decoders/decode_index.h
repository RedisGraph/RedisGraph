/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../graphcontext.h"
#include "../../../redismodule.h"
#include "../../../index/index.h"

Index* RdbLoadIndex(RedisModuleIO *rdb);
