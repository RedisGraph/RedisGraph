/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"

/* The Path project operation consume record from child create path value add it to the record and return the Record. */
typedef struct {
	OpBase op;
	QGPath **paths;
	int *record_idx;
} OpPathProject;

OpBase *NewPathProjectOp(const ExecutionPlan *plan, QGPath **paths);

