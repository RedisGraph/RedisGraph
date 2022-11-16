/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../execution_plan.h"

/* Try to optimize an execution plan segment. */
void optimizePlan(ExecutionPlan *plan);

