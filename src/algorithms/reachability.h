/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../graph/graph.h"
#include "../graph/entities/graph_entity.h"

// returns true if there's a path connecting 'src' to 'dest'
bool reachable
(
	NodeID src,     // source node to traverse from
	NodeID dest,    // destination node to reach
	const Graph *g  // graph to traverse
);

