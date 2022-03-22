/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_extensions.h"
#include "../RG.h"
#include "../util/datablock/oo_datablock.h"

// functions declerations - implemented in graph.c
void Graph_FormConnection(Graph *g, NodeID src, NodeID dest, EdgeID edge_id, int r);

inline void Serializer_Graph_MarkEdgeDeleted
(
	Graph *g,
	EdgeID id
) {
	DataBlock_MarkAsDeletedOutOfOrder(g->edges, id);
}

inline void Serializer_Graph_MarkNodeDeleted
(
	Graph *g,
	NodeID id
) {
	DataBlock_MarkAsDeletedOutOfOrder(g->nodes, id);
}

void Serializer_Graph_SetNode
(
	Graph *g,
	NodeID id,
	LabelID *labels,
	uint label_count,
	Node *n
) {
	ASSERT(g);

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->nodes, id);
	en->prop_count  =  0;
	en->properties  =  NULL;
	n->id           =  id;
	n->entity       =  en;
	GrB_Info info;
	UNUSED(info);

	RG_Matrix nl = Graph_GetNodeLabelMatrix(g);
	for(uint i = 0; i < label_count; i ++) {
		LabelID label = labels[i];
		// set label matrix at position [id, id]
		RG_Matrix  M  =  Graph_GetLabelMatrix(g, label);
		GrB_Matrix m  =  RG_MATRIX_M(M);
		info = GrB_Matrix_setElement_BOOL(m, true, id, id);
		ASSERT(info == GrB_SUCCESS);

		// map this label in this node's set of labels
		info = RG_Matrix_setElement_BOOL(nl, id, label);
		ASSERT(info == GrB_SUCCESS);
	}
}

// computes NodeLabelMarix out of label matrices
// NodeLabelMatrix[:i] = diag(LabelMatrix[i])
// must be called once after all virtual keys loaded for perf
void Serializer_Graph_SetNodeLabels
(
	Graph *g
) {
	ASSERT(g);

	GrB_Vector v;
	int node_count          = Graph_RequiredMatrixDim(g);
	int label_count         = Graph_LabelTypeCount(g);
	RG_Matrix node_labels   = Graph_GetNodeLabelMatrix(g);
	GrB_Matrix node_labels_m = RG_MATRIX_M(node_labels);

#if RG_DEBUG
	GrB_Index nvals;
	GrB_Matrix_nvals(&nvals, node_labels_m);
	ASSERT(nvals == 0);
#endif

	GrB_Vector_new(&v, GrB_BOOL, node_count);

	for(int i = 0; i < label_count; i++) {
		RG_Matrix  M  =  Graph_GetLabelMatrix(g, i);
		GrB_Matrix m  =  RG_MATRIX_M(M);

		GxB_Vector_diag(v, m, 0, NULL);

		GxB_Row_subassign(node_labels_m, NULL, NULL, v, i, GrB_ALL, 0, NULL);
	}

	GrB_transpose(node_labels_m, NULL, NULL, node_labels_m, NULL);

	GrB_Vector_free(&v);
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

// set a given edge in the graph - Used for deserialization of graph
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

// returns the graph deleted nodes list
uint64_t *Serializer_Graph_GetDeletedNodesList
(
	Graph *g
) {
	return g->nodes->deletedIdx;
}

// returns the graph deleted edge list
uint64_t *Serializer_Graph_GetDeletedEdgesList
(
	Graph *g
) {
	return g->edges->deletedIdx;
}

