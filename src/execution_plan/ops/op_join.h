/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
} OpJoin;

OpBase *NewJoinOp(const ExecutionPlan *plan);
