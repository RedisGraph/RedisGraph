/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../resultset/resultset_statistics.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../ast/ast_build_op_contexts.h"

// Context representing a single update to perform on an entity.
typedef struct {
	GraphEntity *ge;                // Entity to be updated.
	int label_id;                   // Label ID if the updated entity is a node.
	bool update_index;              // Whether an index is affected by update.
	SIValue new_value;              // Constant value to set.
	Attribute_ID attr_id;           // ID of attribute to update.
} PendingUpdateCtx;

typedef struct {
	OpBase op;
	GraphContext *gc;
	ResultSetStatistics *stats;

	rax *update_ctxs;               // Entities to update and their expressions.
	PendingUpdateCtx *updates;      // Enqueued updates
	Record *records;                // Updated records, used only when query hands off records after updates.
	bool updates_commited;          // True if we've already committed updates and are now in handoff mode.
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan, rax *update_exps);

