/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/qg_node.h"
#include "../graph/entities/qg_edge.h"
#include <limits.h>

/* Perform DFS scan from node S,
 * Returns a single path from S to a reachable node at distance level. */
QGEdge **DFS(
	QGNode *s,          // Node from which DFS scan begins.
	int level,          // Stop scanning once reached level.
    bool close_cycle    // Allow DFS scan to close a cycle.
);

