/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "runtime_execution_plan.h"

/* Clones an execution plan */
RT_ExecutionPlan *RT_ExecutionPlan_Clone(const RT_ExecutionPlan *plan);
