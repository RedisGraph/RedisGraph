/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* Deletes entities specified within the DELETE clause. */
typedef struct {
	OpBase op;
	AR_ExpNode **exps;      // Expressions evaluated to an entity about to be deleted.
	uint exp_count;         // Number of expressions.
} OpDelete;

OpBase *NewDeleteOp(const ExecutionPlan *plan, AR_ExpNode **exps);
