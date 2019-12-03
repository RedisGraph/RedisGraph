/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
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

// Context describing a pending update to perform.
typedef struct {
	const char *attribute;              /* Attribute name to update. */
	Attribute_ID attr_id;               /* ID of attribute to update. */
	Node n;
	Edge e;
	GraphEntityType entity_type;        /* Graph entity type. */
	SIValue new_value;                  /* Constant value to set. */
} EntityUpdateCtx;

typedef struct {
	OpBase op;
	GraphContext *gc;
	ResultSetStatistics *stats;

	uint update_expressions_count;
	EntityUpdateEvalCtx
	*update_expressions;    /* List of entities to update and their arithmetic expressions. */

	uint pending_updates_cap;
	uint pending_updates_count;
	EntityUpdateCtx
	*pending_updates;           /* List of entities to update and their actual new value. */
	Record *records;                            /* Updated records, used only when query inspects updated entities. */
	bool updates_commited;                      /* Updates performed? */
} OpUpdate;

OpBase *NewUpdateOp(const ExecutionPlan *plan, EntityUpdateEvalCtx *update_exps,
					ResultSetStatistics *stats);

