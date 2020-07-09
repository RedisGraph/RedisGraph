/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "traverse_functions.h"
#include "../../../query_ctx.h"

// Collect edges between the source and destination nodes.
static void _Traverse_CollectEdges(EdgeTraverseCtx *edge_ctx, NodeID src, NodeID dest) {
	Graph *g = QueryCtx_GetGraph();
	uint count = array_len(edge_ctx->edgeRelationTypes);
	for(uint i = 0; i < count; i++) {
		Graph_GetEdgesConnectingNodes(g,
									  src,
									  dest,
									  edge_ctx->edgeRelationTypes[i],
									  &edge_ctx->edges);
	}
}

/* Collects traversed edge relations.
 * e.g. [e:R0|R1]
 * edge_ctx->edgeRelationTypes will hold both R0 and R1 IDs.
 * in the case where no relationship types are specified
 * edge_ctx->edgeRelationTypes will contain GRAPH_NO_RELATION. */
static void _Traverse_SetRelationTypes(EdgeTraverseCtx *edge_ctx, QGEdge *e) {
	uint reltype_count = array_len(e->reltypeIDs);
	if(reltype_count > 0) {
		array_clone(edge_ctx->edgeRelationTypes, e->reltypeIDs);
	} else {
		edge_ctx->edgeRelationTypes = array_new(int, 1);
		edge_ctx->edgeRelationTypes = array_append(edge_ctx->edgeRelationTypes, GRAPH_NO_RELATION);
	}
}

// Determine the edge directions we need to collect.
static GRAPH_EDGE_DIR _Traverse_SetDirection(const AlgebraicExpression *ae, const QGEdge *e) {
	// Bidirectional traversals should match both incoming and outgoing edges.
	if(e->bidirectional) return GRAPH_EDGE_DIR_BOTH;

	/* If this operation traverses a transposed edge, the source and destination
	 * nodes will be swapped in the Record. */
	if(AlgebraicExpression_ContainsOp(ae, AL_EXP_TRANSPOSE)) return GRAPH_EDGE_DIR_INCOMING;

	// The default traversal direction is outgoing.
	return GRAPH_EDGE_DIR_OUTGOING;
}

EdgeTraverseCtx *Traverse_NewEdgeCtx(AlgebraicExpression *ae, QGEdge *e, int idx) {
	EdgeTraverseCtx *edge_ctx = rm_malloc(sizeof(EdgeTraverseCtx));
	edge_ctx->edges = array_new(Edge, 32); // Instantiate array to collect matching edges.
	_Traverse_SetRelationTypes(edge_ctx, e); // Build the array of relation type IDs.
	edge_ctx->edgeRecIdx = idx;
	edge_ctx->direction = _Traverse_SetDirection(ae, e);
	return edge_ctx;
}

/* Populate the traverse context's edges array with all edges of the appropriate
 * direction connecting the source and destination nodes. */
void Traverse_CollectEdges(EdgeTraverseCtx *edge_ctx, NodeID src, NodeID dest) {
	switch(edge_ctx->direction) {
	case GRAPH_EDGE_DIR_OUTGOING:
		_Traverse_CollectEdges(edge_ctx, src, dest);
		return;
	case GRAPH_EDGE_DIR_INCOMING:
		// If we're traversing incoming edges, swap the source and destination.
		_Traverse_CollectEdges(edge_ctx, dest, src);
		return;
	case GRAPH_EDGE_DIR_BOTH:
		// If we're traversing in both directions, collect edges in both directions.
		_Traverse_CollectEdges(edge_ctx, src, dest);
		_Traverse_CollectEdges(edge_ctx, dest, src);
		return;
	}
}

bool Traverse_SetEdge(EdgeTraverseCtx *edge_ctx, Record r) {
	// Return false if all edges have been consumed.
	if(!array_len(edge_ctx->edges)) return false;

	// Pop an edge and add it to the Record.
	Edge e = array_pop(edge_ctx->edges);
	Record_AddEdge(r, edge_ctx->edgeRecIdx, e);
	return true;
}

void Traverse_ResetEdgeCtx(EdgeTraverseCtx *edge_ctx) {
	array_clear(edge_ctx->edges);
}

void Traverse_FreeEdgeCtx(EdgeTraverseCtx *edge_ctx) {
	if(!edge_ctx) return;

	array_free(edge_ctx->edges);
	array_free(edge_ctx->edgeRelationTypes);
	rm_free(edge_ctx);
}

