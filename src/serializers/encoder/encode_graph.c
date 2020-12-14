/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_graph.h"
#include "v8/encode_v8.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	return RdbSaveGraph_v8(rdb, value);
}

