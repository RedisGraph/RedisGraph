/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* Reduce traversal searches for traversal operations where
 * both the src and destination nodes in the traversal are already
 * resolved by former operation, in which case we need to make sure
 * src is connected to dest via the current expression.
 *
 * Consider the following query, execution plan:
 * MATCH (A)-[X]->(B)-[Y]->(A) RETURN A,B
 * SCAN (A)
 * TRAVERSE-1 (A)-[X]->(B)
 * TRAVERSE-2 (B)-[Y]->(A)
 * TRAVERSE-2 tries to see if B is connected to A via Y
 * but A and B are known, we just need to make sure there's an edge
 * of type Y connecting B to A
 * this is done by the EXPAND-INTO operation. */
void reduceTraversal(ExecutionPlan *plan);
