/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"
#include "../../index/index.h"
#include "../ops/ops.h"

/* The utilizeIndices optimization finds Label Scan operations with Filter parents and, if
 * any constant predicate filter matches a viable index, replaces the Label Scan and Filter
 * with an Index Scan. This allows for the consideration of fewer candidate nodes and
 * significantly increases the speed of the operation. */
void utilizeIndices(ExecutionPlan *plan);
