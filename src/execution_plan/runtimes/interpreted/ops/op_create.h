/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../ops/shared/create_functions.h"

/* Creates new entities according to the CREATE clause. */

typedef struct {
	RT_OpBase op;                 // The base operation.
	Record *records;           // Array of Records created by this operation.
	PendingCreations pending;  // Container struct for all graph changes to be committed.
} RT_OpCreate;

RT_OpBase *RT_NewCreateOp(const RT_ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges);
