/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../ast/ast_shared.h"

/* Create new graph entities without introducing any non-unique patterns. */
typedef struct {
	OpBase op;    // The base operation.
	NodeCreateCtx *nodes;
	EdgeCreateCtx *edges;
} OpMergeCreate;

OpBase *NewMergeCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges);
