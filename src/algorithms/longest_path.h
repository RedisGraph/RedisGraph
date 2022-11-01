/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stdint.h>

// Forward declaration
struct QGNode;
struct QueryGraph;

// Determines the longest path length within a tree structured graph.
QGNode *LongestPathTree(const QueryGraph *g, int *level);
// Determines the longest path length within a graph (containing cycles).
QGNode *LongestPathGraph(const QueryGraph *g, int *level);
