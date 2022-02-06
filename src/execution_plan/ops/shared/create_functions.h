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

	Entity *node_properties;
	Entity *edge_properties;

	int **node_labels;
	Node **created_nodes;
	Edge **created_edges;
	ResultSetStatistics *stats;
} PendingCreations;

// Initialize all variables for storing pending creations.
PendingCreations NewPendingCreationsContainer(NodeCreateCtx *nodes, EdgeCreateCtx *edges);

// Lock the graph and commit all changes introduced by the operation.
void CommitNewEntities(OpBase *op, PendingCreations *pending);

// Resolve the properties specified in the query into constant values.
void ConvertPropertyMap(Entity *entity, Record r, PropertyMap *map, bool fail_on_null);

// Free all data associated with a completed create operation.
void PendingCreationsFree(PendingCreations *pending);
