/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* If Optional's child produces records, Optional emits them without modification.
 * If the child produces no records, Optional emits an empty Record exactly once. */
typedef struct {
	OpBase op;
	bool emitted_record;
} Optional;

/* Creates a new Optional operation. */
OpBase *NewOptionalOp(const ExecutionPlan *plan);

