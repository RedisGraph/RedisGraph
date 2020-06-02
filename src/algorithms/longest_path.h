/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/query_graph.h"
#include "../graph/entities/qg_node.h"

// Determins the longest path length within a tree structured graph.
QGNode *LongestPathTree(const QueryGraph *g, int *level);
// Determins the longest path length within a graph (containing cycles).
QGNode *LongestPathGraph(const QueryGraph *g, int *level);
