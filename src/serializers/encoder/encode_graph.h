/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../serializers_include.h"

// graph encoder used by the GRAPH.COPY command to clone a graph
void PipeSaveGraph
(
	Pipe *pipe,  // pipe to write graph binary representation to
	void *value  // graph to encode
);

// graph encoder used by Redis to produce RDB
void RdbSaveGraph
(
	RedisModuleIO *rdb,  // RDB io to write graph binary representation to
	void *value          // graph to encode
);

