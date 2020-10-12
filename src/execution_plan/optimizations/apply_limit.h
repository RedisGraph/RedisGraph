/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* applyLimit will traverse given execution plan looking for limit operations
 * once one is found, all relevant child operations (e.g. sort) will be
 * notify about the current limit
 * this is benifical as a number of different optimizations can be applied 
 * once a limit is known */
void applyLimit(ExecutionPlan *plan);

