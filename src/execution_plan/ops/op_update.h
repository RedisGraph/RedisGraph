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

// Context grouping a set of updates to perform on a single entity
typedef struct {
	union {
		Node n;
		Edge e;
	};								// updated entity
	GraphEntityType entity_type;	// type of updated entity
	bool update_index;				// does index effected by update
	SIValue new_value;              // constant value to set
	Attribute_ID attr_id;			// id of attribute to update
} PendingUpdateCtx;

typedef struct {
	int record_idx;             // record offset this entity is stored at
	const char *alias;			// updated entity alias
	uint nexp;					// number of update expressions
	EntityUpdateEvalCtx *exps;	// list of update expressions
	PendingUpdateCtx *updates;	// list of pending updates
} EntityUpdateCtx;

typedef struct {
	OpBase op;
	GraphContext *gc;
	ResultSetStatistics *stats;

	EntityUpdateCtx *update_ctxs;	// List of entities to update and their arithmetic expressions
	Record *records;                // Updated records, used only when query inspects updated entities
	bool updates_commited;          // Updates performed?
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan, EntityUpdateEvalCtx *update_exps);

