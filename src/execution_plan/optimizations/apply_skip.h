/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* applySkip will traverse the given execution plan looking for Skip operations.
 * Once one is found, all relevant child operations (e.g. Sort) will be
 * notified about the current skip value.
 * This is beneficial as a number of different optimizations can be applied
 * once a skip is known */
void applySkip(ExecutionPlan *plan);

