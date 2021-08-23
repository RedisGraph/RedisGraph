/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"

// The Argument operation holds an internal Record that it will emit exactly once
typedef struct {
	OpBase op;
	Record r;
} RT_Argument;

RT_OpBase *RT_NewArgumentOp(const RT_ExecutionPlan *plan, const char **variables);

void Argument_AddRecord(RT_Argument *arg, Record r);
