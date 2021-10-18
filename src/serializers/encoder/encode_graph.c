/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_graph.h"
#include "v10/encode_v10.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	return RdbSaveGraph_v10(rdb, value);
}

