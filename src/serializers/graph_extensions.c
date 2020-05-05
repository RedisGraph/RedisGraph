/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_extensions.h"
#include "../util/datablock/oo_datablock.h"
#include <assert.h>

// Functions declerations - implemented in graph.c
void Graph_FormConnection(Graph *g, NodeID src, NodeID dest, EdgeID edge_id, int r);

inline void Serializer_Graph_MarkEdgeDeleted(Graph *g, EdgeID id) {
	DataBlock_MarkAsDeletedOutOfOrder(g->edges, id);
}

inline void Serializer_Graph_MarkNodeDeleted(Graph *g, NodeID id) {
	DataBlock_MarkAsDeletedOutOfOrder(g->nodes, id);
}

void Serializer_Graph_SetNode(Graph *g, NodeID id, int label, Node *n) {
	assert(g);

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->nodes, id);
	en->id = id;
	en->prop_count = 0;
	en->properties = NULL;
	n->entity = en;
	if(label != GRAPH_NO_LABEL) {
		// Set matrix at position [id, id]
		GrB_Matrix m = Graph_GetLabelMatrix(g, label);
		GrB_Matrix_setElement_BOOL(m, true, id, id);
	}
}

// Set a given edge in the graph - Used for deserialization of graph.
void Serializer_Graph_SetEdge(Graph *g, EdgeID edge_id, NodeID src, NodeID dest, int r, Edge *e) {
	GrB_Info info;

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->edges, edge_id);
	en->id = edge_id;
	en->prop_count = 0;
	en->properties = NULL;
	e->entity = en;
	e->relationID = r;
	e->srcNodeID = src;
	e->destNodeID = dest;
	Graph_FormConnection(g, src, dest, edge_id, r);
}


// Returns the graph deleted nodes list.
uint64_t *Serializer_Graph_GetDeletedNodesList(Graph *g) {
	return g->nodes->deletedIdx;
}

// Returns the graph deleted nodes list.
uint64_t *Serializer_Graph_GetDeletedEdgesList(Graph *g) {
	return g->edges->deletedIdx;
}