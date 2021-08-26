/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* The Argument operation holds an internal Record that it will emit exactly once. */
typedef struct {
	OpBase op;
} Argument;

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables);
