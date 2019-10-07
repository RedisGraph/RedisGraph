/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __SELECT_ENTRY_POINT_H__
#define __SELECT_ENTRY_POINT_H__

#include "../../filter_tree/filter_tree.h"
#include "../../arithmetic/algebraic_expression.h"

/* The select entry point optimizer inspects an algebraic expression E
 * which will be used shortly for traversal and determins if
 * it would be worth to transpose it, we will choose to transpose if
 * the rows of E have a filter applied to them
 * and the columns of E aren't filtered.
 * As a result of transposing rows will be come columns
 * and we'll be able to perform filtering much quicker. */
void selectEntryPoint(AlgebraicExpression *ae, const FT_FilterNode *tree);

#endif
