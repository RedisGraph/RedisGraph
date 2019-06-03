/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __TRAVERSE_ORDER_H__
#define __TRAVERSE_ORDER_H__

#include "../execution_plan.h"
#include "../../filter_tree/filter_tree.h"
#include "../../arithmetic/algebraic_expression.h"

/* Reorders exps such that exp[i] is the ith expression to evaluate. */
void orderExpressions(AlgebraicExpression **exps, uint exps_count);

typedef enum {
    TRAVERSE_ORDER_FIRST,
    TRAVERSE_ORDER_LAST,
} TRAVERSE_ORDER;

/* Traverse order tries to determine which of the linear expressions should 
 * be used as the first traverse operation, we will prefer using an expression
 * which has a filter applied to it, as we wish to filter as early as we can,
 * that way we expect the number of entities inspected to be reduced at an early stage. */
TRAVERSE_ORDER determineTraverseOrder(const FT_FilterNode *filterTree,
                                      AlgebraicExpression **exps,
                                      size_t expCount);

#endif
