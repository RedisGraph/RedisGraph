/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"
#include "../../filter_tree/filter_tree.h"
#include "../../arithmetic/algebraic_expression.h"
#include "../runtimes/interpreted/runtime_execution_plan.h"

/* Reorders exps such that exp[i] is the ith expression to evaluate. */
void orderExpressions(
	QueryGraph *qg,                 // QueryGraph containing expression entity data.
	AlgebraicExpression **exps,     // Expressions to order.
	uint exps_count,                // Number of expressions.
	const FT_FilterNode *filters,   // Filters.
	rax *bound_vars                 // Previously-bound variables.
);

void compactFilters(ExecutionPlan *plan);
void reduceScans(ExecutionPlan *plan);
void utilizeIndices(ExecutionPlan *plan);
void seekByID(ExecutionPlan *plan);
void filterVariableLengthEdges(ExecutionPlan *plan);
void reduceCartesianProductStreamCount(ExecutionPlan *plan);
void applyJoin(ExecutionPlan *plan);
void reduceFilters(ExecutionPlan *plan);
void reduceTraversal(ExecutionPlan *plan);
void reduceDistinct(ExecutionPlan *plan);
void reduceCount(ExecutionPlan *plan);

void applyLimit(RT_ExecutionPlan *plan);
void applySkip(RT_ExecutionPlan *plan);
