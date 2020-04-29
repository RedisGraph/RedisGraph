/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_extensions.h"
#include "../util/datablock/oo_datablock.h"
#include <assert.h>

// Functions declerations - implemented in graph.c
void Graph_SetEdgeInMatrix(Graph *g, NodeID src, NodeID dest, EdgeID edge_id, int r);

inline void Serializer_Graph_MarkEdgeDeleted(Graph *g, EdgeID id) {
	assert(g);
	DataBlock_MarkAsDeletedOutOfOrder(g->edges, id);
}

inline void Serializer_Graph_MarkNodeDeleted(Graph *g, NodeID id) {
	assert(g);
	DataBlock_MarkAsDeletedOutOfOrder(g->nodes, id);
}

inline void Serializer_Graph_TryAddLabelMatrix(Graph *g, int label) {
	if(label == GRAPH_NO_LABEL) return;
	int label_count = Graph_LabelTypeCount(g);
	if(label >= label_count) {
		for(int i = label_count; i <= label; i++) Graph_AddLabel(g);
	}
}

// Try to add relation matrix if not present. Since edges might be deserialized out of order there could be a situation where
// an edge with the label r is deserialized before the edges with the lables r-x...r-1. This function creates, if needed, all lables matrix required until reaching r  - used for graph deserialization.
static inline void Serializer_Graph_TryAddRelationMatrix(Graph *g, int r) {
	if(r == GRAPH_NO_RELATION) return;
	uint relation_count = Graph_RelationTypeCount(g);
	if(r >= relation_count) {
		for(uint i = relation_count; i <= r; i++) Graph_AddRelationType(g);
	}
}

void Serializer_Graph_SetNode(Graph *g, NodeID id, int label, Node *n) {
	assert(g);

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->nodes, id);
	en->id = id;
	en->prop_count = 0;
	en->properties = NULL;
	n->entity = en;
	if(label != GRAPH_NO_LABEL) {
		Serializer_Graph_TryAddLabelMatrix(g, label);
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

	Serializer_Graph_TryAddRelationMatrix(g, r);
	Graph_SetEdgeInMatrix(g, src, dest, edge_id, r);
}
