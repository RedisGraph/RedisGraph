/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/arithmetic_expression.h"

/* OP Unwind */

typedef struct {
	OpBase op;
	SIValue list;         // List which the unwind operation is performed on.
	AR_ExpNode *exp;      // Arithmetic expression (evaluated as an SIArray).
} OpUnwind;

/* Creates a new Unwind operation */
OpBase *NewUnwindOp(const ExecutionPlan *plan, AR_ExpNode *exp);
