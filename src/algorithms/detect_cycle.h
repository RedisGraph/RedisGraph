/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/query_graph.h"

/* Detects if graph contains a cycle. 
 * assuming graph is fully connected. */
bool IsAcyclicGraph(
	const QueryGraph *qg
);
