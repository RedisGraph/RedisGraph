/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../../serializers_include.h"

GraphContext *RdbLoadGraphContext_v11
(
	RedisModuleIO *rdb
);

void RdbLoadNodes_v11
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t node_count
);

void RdbLoadDeletedNodes_v11
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_node_count
);

void RdbLoadEdges_v11
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t edge_count
);

void RdbLoadDeletedEdges_v11
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_edge_count
);

void RdbLoadGraphSchema_v11
(
	RedisModuleIO *rdb,
	GraphContext *gc
);
