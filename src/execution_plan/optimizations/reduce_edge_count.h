/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* The reduceEdgeCount optimization will look for execution plan 
 * performing solely edge counting: total number of edges in the graph,
 * total number of edges from a specific type.
 * The goal is to reduce the costs of the following execution plans,
 * to a single operation
 * 
 * Before:
 * 127.0.0.1:6379> GRAPH.EXPLAIN g "MATCH ()-[r]->() RETURN COUNT(r)"
 * 1) "Results"
 * 2) "    Aggregate"
 * 3) "        Conditional Traverse"
 * 4) "            All Node Scan"
 * 
 * 
 * 127.0.0.1:6379> GRAPH.EXPLAIN g "MATCH ()-[r:R]->() RETURN COUNT(r)"
 * 1) "Results"
 * 2) "    Aggregate"
 * 3) "        Conditional Traverse"
 * 4) "            All Node Scan"
 * 
 * After:
 * 127.0.0.1:6379> GRAPH.EXPLAIN g "MATCH ()-[r]->() RETURN COUNT(r)"
 * 1) "Results"
 * 2) "    Project" */

void reduceEdgeCount(ExecutionPlan *plan, AST *ast);
