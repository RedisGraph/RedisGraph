/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "encode_graph.h"
#include "v9/encode_v9.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	return RdbSaveGraph_v9(rdb, value);
}

