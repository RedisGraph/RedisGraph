/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"
#include "../execution_plan.h"
#include "../../filter_tree/filter_tree.h"

/* The SelectOrSemiApply operator tests for the presence of a pattern predicate and evaluates a predicate,
 * and is a variation of the Apply operator.
 * This operator allows for the mixing of normal predicates and pattern predicates that check for the presence of a pattern.
 * First, the normal expression predicate is evaluated,
 * and, only if it returns false, is the costly pattern predicate evaluated. */
typedef struct {
	OpBase op;
    Record r;                   // Lefthand-side record.
    bool anti;                  // AntiSelectOrSemiApply.
    Argument *op_arg;           // Righthand-side tap.
    FT_FilterNode *filter;      // "Cheap" filter to apply.
} SelectOrSemiApply;

OpBase *NewSelectOrSemiApplyOp(ExecutionPlan *plan, FT_FilterNode *filter, bool anti);
