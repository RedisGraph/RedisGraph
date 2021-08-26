/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../ast/ast_shared.h"

/* Creates new entities according to the CREATE clause. */

typedef struct {
	OpBase op;                 // The base operation.
	NodeCreateCtx *nodes;
	EdgeCreateCtx *edges;
} OpCreate;

OpBase *NewCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges);
