/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../execution_plan.h"

/* The seek by ID optimization searches for a SCAN operation on which 
 * a filter of the form ID(n) = X is applied in which case 
 * both the SCAN and FILTER operations can be reduced into a single 
 * NODE_BY_ID_SEEK operation. */
void seekByID(ExecutionPlanSegment *plan);
