/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../IR/execution_plan/execution_plan.h"
#include "../../ast/ast_shared.h"
#include "shared/create_functions.h"
#include "../../storage/entities/node.h"
#include "../../storage/entities/edge.h"

/* Creates new entities according to the CREATE clause. */

typedef struct {
	OpBase op;                 // The base operation.
	Record *records;           // Array of Records created by this operation.
	PendingCreations pending;  // Container struct for all graph changes to be committed.
} OpCreate;

OpBase *NewCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges);
