/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"

/* Merge all order expressions that can operate on the input Record into the projections array,
 * removing duplicates. The ORDER array will be set to NULL if it is depleted, otherwise it will
 * contain expressions that must be evaluated against an intermediate Record. */
void CombineProjectionArrays(AR_ExpNode ***project_exp_ptr, AR_ExpNode ***order_exp_ptr);

