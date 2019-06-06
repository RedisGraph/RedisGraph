/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"
#include "../../filter_tree/filter_tree.h"
#include "../../arithmetic/algebraic_expression.h"

/* Reorders exps such that exp[i] is the ith expression to evaluate. */
void orderExpressions(AlgebraicExpression **exps, uint exps_count);
