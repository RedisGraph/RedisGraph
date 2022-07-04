/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../encode_io.h"
#include "../../serializers_include.h"

void RdbSaveGraph_v12
(
	IOEncoder *io,
	void *value
);

void RdbSaveNodes_v12
(
	IOEncoder *io,
	GraphContext *gc,
	uint64_t nodes_to_encode
);

void RdbSaveDeletedNodes_v12
(
	IOEncoder *io,
	GraphContext *gc,
	uint64_t deleted_nodes_to_encode
);

void RdbSaveEdges_v12
(
	IOEncoder *io,
	GraphContext *gc,
	uint64_t edges_to_encode
);

void RdbSaveDeletedEdges_v12
(
	IOEncoder *io,
	GraphContext *gc,
	uint64_t deleted_edges_to_encode
);

void RdbSaveGraphSchema_v12
(
	IOEncoder *io,
	GraphContext *gc
);

