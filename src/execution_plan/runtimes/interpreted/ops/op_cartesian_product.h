/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/op_cartesian_product.h"

/* Cartesian product AKA Join. */
typedef struct {
	RT_OpBase op;
	const CartesianProduct *op_desc;
	Record r;
	bool init;
} RT_CartesianProduct;

RT_OpBase *RT_NewCartesianProductOp(const RT_ExecutionPlan *plan, const CartesianProduct *op_desc);
