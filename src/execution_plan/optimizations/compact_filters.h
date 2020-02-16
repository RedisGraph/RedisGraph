/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* The compact filters optimizer scans an execution plans for filters that can be
 * compressed for either 'true' or 'false' final values. In case the filter is reduced
 * to 'true' the filter operation will be removed from the execution plan. */
void compactFilters(ExecutionPlan *plan);
