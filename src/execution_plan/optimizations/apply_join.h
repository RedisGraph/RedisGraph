/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* applyJoin will try to locate situations where two disjoint
 * streams can be joined on a key attribute, in which case the
 * runtime complaxity is reduced from O(n^2) to O(nlogn + 2n)
 * consider MATCH (a), (b) where a.v = b.v RETURN a,b
 * prior to this optimization a and b will be combined via a
 * cartesian product O(n^2) because a and b are related,
 * we require a.v = b.v, v acts as a join key in which case
 * replacing the cartesian product by a join operation will
 * 1. consume N additional memory
 * 2. reduce the overall runtime by a factor of magnitude. */
void applyJoin(ExecutionPlan *plan);
