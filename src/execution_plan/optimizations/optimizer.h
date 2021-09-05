/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"
#include "../runtimes/interpreted/runtime_execution_plan.h"

/* Try to optimize an execution plan segment. */
void optimizePlan(ExecutionPlan *plan);
void optimize_RTPlan(RT_ExecutionPlan *plan);
