/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"

// The keys from the "graphmeta" type are virtual keys
// that created upon RDB save and deleted upon RDB or replication process ending
// each for those keys RDB value represent an independent part of the graph
// that can be encoded/decoded as is
// this allows saving and replicating large graphs with small memory overhead

int GraphMetaType_Register
(
	RedisModuleCtx *ctx
);

