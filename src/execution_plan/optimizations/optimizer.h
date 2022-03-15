/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* Try to optimize an execution plan segment. */
void optimizePlan(ExecutionPlan *plan);

