/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* Cartesian product AKA Join. */
typedef struct {
	OpBase op;
	Record r;
	bool init;
} CartesianProduct;

OpBase *NewCartesianProductOp(const ExecutionPlan *plan);
