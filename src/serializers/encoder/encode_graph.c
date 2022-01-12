/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_graph.h"
#include "v11/encode_v11.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	RdbSaveGraph_v11(rdb, value);
}

