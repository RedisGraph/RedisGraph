/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"

#include <limits.h>

/* Perform DFS scan from node S,
 * Returns a single path from S to a reachable node at distance level. */
Edge** DFS (
    Node *s,    // Node from which DFS scan begins.
    int level   // Stop scanning once reached level.
);
