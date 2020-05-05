/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "serializers_include.h"

/* The keys from the "graphmeta" type are virtual keys that created upon RDB save
 * and deleted upon RDB or replication process ending.
 * Each for those keys RDB value represent an independent part of the graph that can be encoded/decoded as is.
 * This allows saving and replicating large graphs with small memory overhead */

int GraphMetaType_Register(RedisModuleCtx *ctx);
