/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "execution_plan.h"

/* Clones an execution plan */
ExecutionPlan *ExecutionPlan_Clone(const ExecutionPlan *plan);

