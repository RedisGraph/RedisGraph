/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __TRANSLATE_FILTERS_H__
#define __TRANSLATE_FILTERS_H__

#include "../execution_plan.h"

/* Translate filters tries to transform filters operations from their original representation (filter tree)
 * into a matrix representation, this is possible when a filter tree F deals with a single entity
 * for example: F = (P.age > 33) in that case F's filter operation is removed and F is converted into a matrix
 * representation attached to a traverse operation.
 *
 * Gain: 1. less operations to deal with.
 *       2. early reduction of the search space. 
 *       3. removal of intermidate nodes, enable reduction of traversal operations. */
void translateFilters(ExecutionPlan *plan);

#endif
