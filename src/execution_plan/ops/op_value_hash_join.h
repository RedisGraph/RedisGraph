/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

typedef struct {
	OpBase op;
	AR_ExpNode *lhs_exp;                // Left hand side expression to join on.
	AR_ExpNode *rhs_exp;                // Right hand side expression to join on.
} OpValueHashJoin;

/* Creates a new ValueHashJoin operation */
OpBase *NewValueHashJoin(const ExecutionPlan *plan, AR_ExpNode *lhs_exp, AR_ExpNode *rhs_exp);
