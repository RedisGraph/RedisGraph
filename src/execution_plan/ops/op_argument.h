/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* The Argument operation holds an internal Record that it will emit exactly once. */
typedef struct {
	OpBase op;
	Record r;     // record to emit
	Record reset; // record to be restored to 'r' in the event of ArgumentReset
} Argument;

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables);

void Argument_AddRecord(Argument *arg, Record r);

