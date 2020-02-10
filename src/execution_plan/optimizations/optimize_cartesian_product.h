/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../execution_plan.h"

/* This optimization takes multiple branched cartesian product (with more than two branches),
 * followed by filter(s) and try to apply the filter as soon possible by
 * locating situations where a new Cartesian Product of smaller amount of streams can resolve the filter.
 * For a filter F executing on a dual-branched cartesian product output, the runtime complexity is at most f=n^2.
 * For a filter F' which execute on a dual-branched cartesian product output, where one of its branches is F,
 * the overall time complexity is at most f'=f*n = n^3.
 * In the general case, the runtime complaxity of filter that is executing over the output of a cartesian product
 * which all of its children are nested cartesian product followed by a filter (as a result of this optimization)
 * is at most n^x where x is the number of branchs of the original cartesian product.
 * Consider MATCH (a), (b), (c) where a.x > b.x RETURN a, b, c
 * Prior to this optimization a, b and c will be combined via a cartesian product O(n^3).
 * Because we require a.v > b.v we can create a cartesian product between
 * a and b, and re-position the filter after this new cartesian product, remove both a and b branches from
 * the original cartesian product and place the filter operation is a new branch.
 * Creating nested cartesian products operations and re-positioning the filter op will:
 * 1. Potentially reduce memory consumption (storing only f records instead n^x) in each phase.
 * 2. Reduce the overall filter runtime by potentially order(s) of magnitude. */
void reduceCartesianProductStreamCount(ExecutionPlan *plan);
