/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../execution_plan.h"
#include "../../../arithmetic/algebraic_expression.h"

/* Container struct for traversing and populating referenced edges in
 * traversal ops like CondTraverse and ExpandInto. */
typedef struct {
	int *edgeRelationTypes;     // The relation type IDs that should be collected.
	Edge *edges;                // Flexible array of all matching edges for the current endpoints.
	int edgeRecIdx;             // The Record index for the referenced edge.
	GRAPH_EDGE_DIR direction;   // The direction of the referenced edge being traversed.
} EdgeTraverseCtx;

// Initialize an EdgeTraverseCtx struct to populate edges appropriately for traversal operations.
EdgeTraverseCtx *Traverse_NewEdgeCtx(AlgebraicExpression *ae, QGEdge *e, int idx);

// Collect all appropriate edges between the given endpoints.
void Traverse_CollectEdges(EdgeTraverseCtx *edge_ctx, NodeID src, NodeID dest);

// Remove a matching edge from the edges array if one is available and set it in the Record.
bool Traverse_SetEdge(EdgeTraverseCtx *edge_ctx, Record r);

// Reset contained edges within an EdgeTraverseCtx.
void Traverse_ResetEdgeCtx(EdgeTraverseCtx *edge_ctx);

// Free an EdgeTraverseCtx struct.
void Traverse_FreeEdgeCtx(EdgeTraverseCtx *edge_ctx);

