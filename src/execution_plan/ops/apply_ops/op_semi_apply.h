/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../op.h"
#include "../op_argument.h"
#include "../../execution_plan.h"

struct OpSemiApply;

typedef Record(*ApplyLogic)(struct OpSemiApply *);

/* SemiApply operation tests for the presence of a pattern
 * It starts by pulling on the left-hand side branch,
 * for each record received it tries to get a record from the right-hand side
 * if no data is produced it will try to fetch a new data point from the left-hand side
 * otherwise the left-hand side record is passed onward. */
typedef struct OpSemiApply {
	OpBase op;
	Record r;               // Lefthand-side record.
	Argument *op_arg;       // Righthand-side tap.
	ApplyLogic apply_func;  // Which apply method to invoke (semi or anti semi apply)
} OpSemiApply;

OpBase *NewSemiApplyOp(ExecutionPlan *plan, bool anti);
