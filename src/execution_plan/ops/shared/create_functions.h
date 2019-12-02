/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"
#include "../../../ast/ast_shared.h"

// Container struct for properties to be added to a new graph entity
typedef struct {
	const Attribute_ID *attr_keys; // IDs of property keys to be added.
	SIValue *values;               // Property values associated with each ID.
	int property_count;            // Number of properties to be added.
} PendingProperties;

typedef struct {
	NodeCreateCtx *nodes_to_create;
	EdgeCreateCtx *edges_to_create;

	PendingProperties **node_properties;
	PendingProperties **edge_properties;

	Node **created_nodes;
	Edge **created_edges;
	ResultSetStatistics *stats;
} PendingCreations;

// Initialize all variables for storing pending creations.
PendingCreations NewPendingCreationsContainer(ResultSetStatistics *stats, NodeCreateCtx *nodes,
											  EdgeCreateCtx *edges);

// Lock the graph and commit all changes introduced by the operation.
void CommitNewEntities(PendingCreations *pending);

// Resolve the properties specified in the query into constant values.
PendingProperties *ConvertPropertyMap(Record r, const PropertyMap *map);

// Free all data associated with a PendingProperties container.
void PendingPropertiesFree(PendingProperties *props);

// Free all data associated with a completed create operation.
void PendingCreationsFree(PendingCreations *pending);

