/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../../serializers_include.h"

void RdbSaveGraph_v12
(
	RedisModuleIO *rdb,
	void *value
);

void RdbSaveNodes_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t nodes_to_encode
);

void RdbSaveDeletedNodes_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_nodes_to_encode
);

void RdbSaveEdges_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t edges_to_encode
);

void RdbSaveDeletedEdges_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_edges_to_encode
);

void RdbSaveGraphSchema_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc
);

