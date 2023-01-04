/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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

