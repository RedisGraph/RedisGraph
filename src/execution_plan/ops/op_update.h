/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "shared/update_functions.h"
#include "../../resultset/resultset_statistics.h"

typedef struct {
	OpBase op;
	rax *update_ctxs;               // Entities to update and their expressions
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps);
