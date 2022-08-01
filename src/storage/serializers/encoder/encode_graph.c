/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_graph.h"
#include "v12/encode_v12.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	RdbSaveGraph_v12(rdb, value);
}

