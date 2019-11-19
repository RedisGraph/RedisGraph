/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../execution_plan.h"
#include "../../ast/ast_shared.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../resultset/resultset_statistics.h"

/* Creates new entities according to the CREATE clause. */

// Container struct for properties to be added to a new graph entity
typedef struct {
	const Attribute_ID *attr_keys; // IDs of property keys to be added.
	SIValue *values;               // Property values associated with each ID.
	int property_count;            // Number of properties to be added.
} PendingProperties;

typedef struct {
	OpBase op;
	QueryGraph *qg;
	GraphContext *gc;
	Record *records;
	rax *unique_entities;

	NodeCreateCtx *nodes_to_create;
	EdgeCreateCtx *edges_to_create;
	PendingProperties **node_properties;
	PendingProperties **edge_properties;

	Node **created_nodes;
	Edge **created_edges;
	ResultSetStatistics *stats;
} OpCreate;

OpBase *NewCreateOp(const ExecutionPlan *plan, ResultSetStatistics *stats, NodeCreateCtx *nodes,
					EdgeCreateCtx *edges, bool no_duplicate_creations);

