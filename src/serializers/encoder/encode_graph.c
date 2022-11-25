/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "encode_graph.h"
#include "v12/encode_v12.h"

void RdbSaveGraph(RedisModuleIO *rdb, void *value) {
	RdbSaveGraph_v12(rdb, value);
}

