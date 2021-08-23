/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"

/* Cartesian product AKA Join. */
typedef struct {
	RT_OpBase op;
	Record r;
	bool init;
} RT_CartesianProduct;

RT_OpBase *RT_NewCartesianProductOp(const RT_ExecutionPlan *plan);
