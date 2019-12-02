/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../ast/ast_shared.h"
#include "shared/create_functions.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../resultset/resultset_statistics.h"

/* Creates new entities according to the CREATE clause. */

typedef struct {
	OpBase op;                 // The base operation.
	Record *records;           // Array of Records created by this operation.
	PendingCreations pending;  // Container struct for all graph changes to be committed.
} OpCreate;

OpBase *NewCreateOp(const ExecutionPlan *plan, ResultSetStatistics *stats, NodeCreateCtx *nodes,
					EdgeCreateCtx *edges);

