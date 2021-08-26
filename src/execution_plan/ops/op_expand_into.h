/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "shared/traverse_functions.h"
#include "../execution_plan.h"
#include "../../arithmetic/algebraic_expression.h"

typedef struct {
	OpBase op;
	AlgebraicExpression *ae;
} OpExpandInto;

OpBase *NewExpandIntoOp(const ExecutionPlan *plan, AlgebraicExpression *ae);
