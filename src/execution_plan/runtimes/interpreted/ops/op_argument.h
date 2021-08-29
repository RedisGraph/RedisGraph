/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../../../ops/op_argument.h"
#include "../runtime_execution_plan.h"

// The Argument operation holds an internal Record that it will emit exactly once
typedef struct {
	RT_OpBase op;
	const Argument *op_desc;
	Record r;
} RT_Argument;

RT_OpBase *RT_NewArgumentOp(const RT_ExecutionPlan *plan, const Argument *op_desc);

void Argument_AddRecord(RT_Argument *arg, Record r);
