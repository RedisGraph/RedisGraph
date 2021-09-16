/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_extensions.h"
#include "../RG.h"
#include "../util/datablock/oo_datablock.h"

// Functions declerations - implemented in graph.c
void Graph_FormConnection(Graph *g, NodeID src, NodeID dest, EdgeID edge_id, int r);

inline void Serializer_Graph_MarkEdgeDeleted(Graph *g, EdgeID id) {
	DataBlock_MarkAsDeletedOutOfOrder(g->edges, id);
}

inline void Serializer_Graph_MarkNodeDeleted(Graph *g, NodeID id) {
	DataBlock_MarkAsDeletedOutOfOrder(g->nodes, id);
}

void Serializer_Graph_SetNode(Graph *g, NodeID id, int label, Node *n) {
	ASSERT(g);

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->nodes, id);
	en->prop_count = 0;
	en->properties = NULL;
	n->id = id;
	n->entity = en;
	if(label != GRAPH_NO_LABEL) {
		// Set matrix at position [id, id]
		RG_Matrix M   =  Graph_GetLabelMatrix(g, label);
		GrB_Matrix m  =  RG_MATRIX_M(M);
		
		// Optimize set only for decoder
		GrB_Matrix_setElement_BOOL(m, true, id, id);

		// a node with 'label' has just been created, update statistics
		GraphStatistics_IncNodeCount(&g->stats, label, 1);
	}
}

// optimized version of Graph_FormConnection
// used only when matrix not contains multi edge values
static void _OptimizedSingleEdgeFormConnection
(
	Graph *g,
	NodeID src,
	NodeID dest,
	EdgeID edge_id,
	int r
) {
	GrB_Info info;
	RG_Matrix  M      =  Graph_GetRelationMatrix(g, r, false);
	RG_Matrix  adj    =  Graph_GetAdjacencyMatrix(g, false);
	GrB_Matrix m      =  RG_MATRIX_M(M);
	GrB_Matrix tm     =  RG_MATRIX_TM(M);
	GrB_Matrix adj_m  =  RG_MATRIX_M(adj);
	GrB_Matrix adj_tm =  RG_MATRIX_TM(adj);

	UNUSED(info);

	// rows represent source nodes, columns represent destination nodes

	//--------------------------------------------------------------------------
	// update adjacency matrix
	//--------------------------------------------------------------------------

	info = GrB_Matrix_setElement_BOOL(adj_m, true, src, dest);
	ASSERT(info == GrB_SUCCESS);
	info = GrB_Matrix_setElement_BOOL(adj_tm, true, dest, src);
	ASSERT(info == GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// update relationship matrix
	//--------------------------------------------------------------------------

	info = GrB_Matrix_setElement_UINT64(m, edge_id, src, dest);
	ASSERT(info == GrB_SUCCESS);
	info = GrB_Matrix_setElement_UINT64(tm, edge_id, dest, src);
	ASSERT(info == GrB_SUCCESS);

	// an edge of type r has just been created, update statistics
	// TODO: stats->edge_count[relation_idx] += nvals;
	GraphStatistics_IncEdgeCount(&g->stats, r, 1);
}

// Set a given edge in the graph - Used for deserialization of graph.
void Serializer_Graph_SetEdge
(
	Graph *g,
	bool multi_edge,
	EdgeID edge_id,
	NodeID src,
	NodeID dest,
	int r,
	Edge *e
) {
	GrB_Info info;

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->edges, edge_id);
	en->prop_count = 0;
	en->properties = NULL;
	e->id = edge_id;
	e->entity = en;
	e->relationID = r;
	e->srcNodeID = src;
	e->destNodeID = dest;

	if(multi_edge) {
		Graph_FormConnection(g, src, dest, edge_id, r);
	} else {
		_OptimizedSingleEdgeFormConnection(g, src, dest, edge_id, r);
	}
}

// Returns the graph deleted nodes list.
uint64_t *Serializer_Graph_GetDeletedNodesList(Graph *g) {
	return g->nodes->deletedIdx;
}

// Returns the graph deleted nodes list.
uint64_t *Serializer_Graph_GetDeletedEdgesList(Graph *g) {
	return g->edges->deletedIdx;
}

