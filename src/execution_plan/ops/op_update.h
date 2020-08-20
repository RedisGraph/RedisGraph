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

// Context grouping a set of updates to perform on a single entity.
typedef struct {
	union {
		Node n;
		Edge e;
	};                              // Updated entity.
	GraphEntityType entity_type;    // Type of updated entity.
	bool update_index;              // Does index effected by update.
	SIValue new_value;              // Constant value to set.
	Attribute_ID attr_id;           // Id of attribute to update.
} PendingUpdateCtx;

typedef struct {
	int record_idx;             // Record offset this entity is stored at.
	const char *alias;          // Updated entity alias.
	EntityUpdateEvalCtx *exps;  // Update expressions converted from the AST.
	PendingUpdateCtx *updates;  // List of pending updates for this op to commit.
} EntityUpdateCtx;

typedef struct {
	OpBase op;
	GraphContext *gc;
	ResultSetStatistics *stats;

	EntityUpdateCtx *update_ctxs;   // Entities to update and their expressions.
	Record *records;                // Updated records, used only when query hands off records after updates.
	bool updates_commited;          // True if we've already committed updates and are now in handoff mode.
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan,
					EntityUpdateEvalCtx *update_exps);

