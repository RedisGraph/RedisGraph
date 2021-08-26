/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../arithmetic/algebraic_expression.h"

// OP Traverse
typedef struct {
	OpBase op;
	AlgebraicExpression *ae;
	int dest_label_id;          // ID of destination node label if known
	const char *dest_label;     // Label of destination node if known
} OpCondTraverse;

// Creates a new Traverse operation
OpBase *NewCondTraverseOp(const ExecutionPlan *plan, AlgebraicExpression *ae);
