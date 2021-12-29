/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "decode_graph.h"
#include "current/v11/decode_v11.h"

GraphContext *RdbLoadGraph(RedisModuleIO *rdb) {
	return RdbLoadGraphContext_v11(rdb);
}

