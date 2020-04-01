/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"
#include "../../../arithmetic/algebraic_expression.h"

typedef struct {
	int *edgeRelationTypes;     // The relation type IDs that should be collected.
	Edge *edges;                // Flexible array of all matching edges for the current endpoints.
	int edgeIdx;                // The Record index for the referenced edge.
	GRAPH_EDGE_DIR direction;   // The direction of the referenced edge being traversed.
} EdgeTraverseData;

// Initialize an EdgeTraverseData struct to populate edges appropriately for traversal operations.
void Traverse_NewEdgeData(EdgeTraverseData *edge_data, AlgebraicExpression *ae,
						  QGEdge *e, int idx);

// Collect all appropriate edges between the given endpoints.
void Traverse_CollectEdges(EdgeTraverseData *edge_data, NodeID src, NodeID dest);

// If a matching edge is available, pop it and add it to the Record.
bool Traverse_SetEdge(EdgeTraverseData *edge_data, Record r);

// Free an EdgeTraverseData struct.
void Traverse_FreeEdgeData(EdgeTraverseData *edge_data);

