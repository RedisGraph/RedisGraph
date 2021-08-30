/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "../runtime_execution_plan.h"
#include "../../../../ast/ast_shared.h"
#include "../../../ops/op_merge_create.h"
#include "../../../../graph/entities/node.h"
#include "../../../../graph/entities/edge.h"
#include "../../../ops/shared/create_functions.h"

/* Create new graph entities without introducing any non-unique patterns. */
typedef struct {
	RT_OpBase op;              // The base operation.
	const OpMergeCreate *op_desc;
	bool handoff_mode;         // Flag denoting whether the op is in Record creation or handoff mode.
	Record *records;           // Array of Records created by this operation.
	rax *unique_entities;      // A map of each unique pending set of creations.
	XXH64_state_t *hash_state; // Reusable hash state for determining creation uniqueness.
	PendingCreations pending;  // Container struct for all graph changes to be committed.
} RT_OpMergeCreate;

RT_OpBase *RT_NewMergeCreateOp(const RT_ExecutionPlan *plan, const OpMergeCreate *op_desc);

// Commit all pending creations and switch to Record handoff mode.
void MergeCreate_Commit(RT_OpBase *opBase);
