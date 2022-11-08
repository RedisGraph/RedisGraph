/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"
#include "../../../ast/ast_shared.h"
#include "../../../resultset/resultset_statistics.h"

typedef struct {
	NodeCreateCtx *nodes_to_create;
	EdgeCreateCtx *edges_to_create;

	AttributeSet *node_attributes;
	AttributeSet *edge_attributes;

	int **node_labels;
	Node **created_nodes;
	Edge **created_edges;
	ResultSetStatistics *stats;
} PendingCreations;

// initialize all variables for storing pending creations
void NewPendingCreationsContainer
(
	PendingCreations *pending,
	NodeCreateCtx *nodes,
	EdgeCreateCtx *edges
);

// lock the graph and commit all changes introduced by the operation
void CommitNewEntities
(
	OpBase *op,
	PendingCreations *pending
);

// resolve the properties specified in the query into constant values
void ConvertPropertyMap
(
	GraphContext* gc,
	AttributeSet *attributes,
	Record r,
	PropertyMap *map,
	bool fail_on_null
);

// free all data associated with a completed create operation
void PendingCreationsFree
(
	PendingCreations *pending
);

