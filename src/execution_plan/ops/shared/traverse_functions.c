/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "traverse_functions.h"
#include "../../../query_ctx.h"

// Collect edges between the source and destination nodes.
static void _Traverse_CollectEdges(EdgeTraverseData *edge_data, NodeID src, NodeID dest) {
	Graph *g = QueryCtx_GetGraph();
	uint count = array_len(edge_data->edgeRelationTypes);
	for(uint i = 0; i < count; i++) {
		Graph_GetEdgesConnectingNodes(g,
									  src,
									  dest,
									  edge_data->edgeRelationTypes[i],
									  &edge_data->edges);
	}
}

/* Collects traversed edge relations.
 * e.g. [e:R0|R1]
 * edge_data->edgeRelationTypes will hold both R0 and R1 IDs.
 * in the case where no relationship types are specified
 * edge_data->edgeRelationTypes will contain GRAPH_NO_RELATION. */
static void _Traverse_SetRelationTypes(EdgeTraverseData *edge_data, QGEdge *e) {
	uint reltype_count = array_len(e->reltypeIDs);
	if(reltype_count > 0) {
		array_clone(edge_data->edgeRelationTypes, e->reltypeIDs);
	} else {
		edge_data->edgeRelationTypes = array_new(int, 1);
		edge_data->edgeRelationTypes = array_append(edge_data->edgeRelationTypes, GRAPH_NO_RELATION);
	}
}

void Traverse_NewEdgeData(EdgeTraverseData *edge_data, AlgebraicExpression *ae,
						  QGEdge *e, int idx) {
	edge_data->edges = array_new(Edge, 32); // Instantiate array to collect matching edges.
	_Traverse_SetRelationTypes(edge_data, e); // Build the array of relation type IDs.
	edge_data->edgeIdx = idx;
	// Determine the edge directions we need to collect.
	if(e->bidirectional) {
		// Bidirectional edges matching incoming and outgoing edges.
		edge_data->direction = GRAPH_EDGE_DIR_BOTH;
	} else if(AlgebraicExpression_ContainsOp(ae, AL_EXP_TRANSPOSE)) {
		/* If this operation traverses a transposed edge, the source and destination nodes
		 * will be swapped in the Record. */
		edge_data->direction = GRAPH_EDGE_DIR_INCOMING;
	} else {
		// The default traversal direction is outgoing.
		edge_data->direction = GRAPH_EDGE_DIR_OUTGOING;
	}
}

// Collect edges between the source and destination nodes matching the op's traversal direction.
void Traverse_CollectEdges(EdgeTraverseData *edge_data, NodeID src, NodeID dest) {
	switch(edge_data->direction) {
	case GRAPH_EDGE_DIR_OUTGOING:
		_Traverse_CollectEdges(edge_data, src, dest);
		return;
	case GRAPH_EDGE_DIR_INCOMING:
		// If we're traversing incoming edges, swap the source and destination.
		_Traverse_CollectEdges(edge_data, dest, src);
		return;
	case GRAPH_EDGE_DIR_BOTH:
		// If we're traversing in both directions, collect edges in both directions.
		_Traverse_CollectEdges(edge_data, src, dest);
		_Traverse_CollectEdges(edge_data, dest, src);
		return;
	}
}

bool Traverse_SetEdge(EdgeTraverseData *edge_data, Record r) {
	// Return false if all edges have been consumed.
	if(!array_len(edge_data->edges)) return false;

	// Pop an edge and add it to the Record.
	Edge e = array_pop(edge_data->edges);
	Record_AddEdge(r, edge_data->edgeIdx, e);
	return true;
}

void Traverse_FreeEdgeData(EdgeTraverseData *edge_data) {
	if(edge_data->edges) {
		array_free(edge_data->edges);
		edge_data->edges = NULL;
	}

	if(edge_data->edgeRelationTypes) {
		array_free(edge_data->edgeRelationTypes);
		edge_data->edgeRelationTypes = NULL;
	}
}

