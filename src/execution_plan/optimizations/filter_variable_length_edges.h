/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../execution_plan.h"

/* Isolate filters that operate on a traversed variable-length edge
 * and embed them in the traversal op. */
void filterVariableLengthEdges(ExecutionPlan *plan);

