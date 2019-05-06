/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* The reduceCount optimization will look for execution plan 
 * performing solely node counting: total number of nodes in the graph,
 * total number of nodes with a specific label.
 * In which case we can avoid performing both SCAN* and AGGREGATE 
 * operations by simply returning Graph_NodeCount or Graph_LabeledNodeCount. */
void reduceCount(ExecutionPlanSegment *plan);
