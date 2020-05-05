/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_graph.h"
#include "v7/encode_v7.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	return RdbSaveGraph_v7(rdb, value);
}
