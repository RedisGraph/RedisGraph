/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* The Argument operation holds an internal record array and emits one record per invocation.
 * When populating an eager operation like Create, the Argument should hold all necessary records,
 * in other contexts the 'records' array will contain only one record at a time. */
typedef struct {
	OpBase op;
	Record *records;
} Argument;

OpBase *NewArgumentOp(const ExecutionPlan *plan, const char **variables);

void Argument_AddRecord(Argument *arg, Record r);

