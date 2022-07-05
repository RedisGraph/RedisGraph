/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../../serializers_include.h"

GraphContext *RdbLoadGraphContext_v12
(
	IODecoder *io
);

void RdbLoadNodes_v12
(
	IODecoder *io,
	GraphContext *gc,
	uint64_t node_count
);

void RdbLoadDeletedNodes_v12
(
	IODecoder *io,
	GraphContext *gc,
	uint64_t deleted_node_count
);

void RdbLoadEdges_v12
(
	IODecoder *io,
	GraphContext *gc,
	uint64_t edge_count
);

void RdbLoadDeletedEdges_v12
(
	IODecoder *io,
	GraphContext *gc,
	uint64_t deleted_edge_count
);

void RdbLoadGraphSchema_v12
(
	IODecoder *io,
	GraphContext *gc
);

