/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
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

/* Create new graph entities without introducing any non-unique patterns. */
typedef struct {
	OpBase op;                 // The base operation.
	bool handoff_mode;         // Flag denoting whether the op is in Record creation or handoff mode.
	Record *records;           // Array of Records created by this operation.
	rax *unique_entities;      // A map of each unique pending set of creations.
	XXH64_state_t *hash_state; // Reusable hash state for determining creation uniqueness.
	PendingCreations pending;  // Container struct for all graph changes to be committed.
	GraphContext *gc;
} OpMergeCreate;

OpBase *NewMergeCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges);

// Commit all pending creations and switch to Record handoff mode.
void MergeCreate_Commit(OpBase *opBase);
