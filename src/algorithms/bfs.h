/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/qg_node.h"
#include <limits.h>

#define BFS_LOWEST_LEVEL INT_MAX    // Return leaf nodes at deepest level.

/* Perform BFS scan from node S,
 * Returns a set of nodes reached at given level.
 * Incase level is set to BFS_LOWEST_LEVEL
 * well scan all the way to the bottom and set level
 * to indicate the level reached. */
QGNode **BFS(
	QGNode *s,  // Node from which BFS scan begins.
	int *level  // Stop scanning at level.
);
