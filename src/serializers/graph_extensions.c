/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graph_extensions.h"
#include "../util/datablock/oo_datablock.h"
#include <assert.h>

// Extern variables - from graph.c
extern GrB_BinaryOp graph_edge_accum;
// Functions declerations - implemented in graph.c

GrB_Matrix RG_Matrix_Get_GrB_Matrix(RG_Matrix matrix);
void MatrixResizeToCapacity(const Graph *g, RG_Matrix m);
GrB_Matrix Graph_Get_Transposed_AdjacencyMatrix(const Graph *g);

void Graph_SetNode(Graph *g, NodeID id, int label, Node *n) {
	assert(g);

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->nodes, id);
	en->id = id;
	en->prop_count = 0;
	en->properties = NULL;
	n->entity = en;
	if(label != GRAPH_NO_LABEL) {
		Graph_TryAddLabelMatrix(g, label);
		// Try to set matrix at position [id, id]
		// incase of a failure, scale matrix.
		RG_Matrix matrix = g->labels[label];
		GrB_Matrix m = RG_Matrix_Get_GrB_Matrix(matrix);
		GrB_Info res = GrB_Matrix_setElement_BOOL(m, true, id, id);
		if(res != GrB_SUCCESS) {
			MatrixResizeToCapacity(g, matrix);
			assert(GrB_Matrix_setElement_BOOL(m, true, id, id) == GrB_SUCCESS);
		}
	}
}

// Try to add relation matrix if not present. Since edges might be deserialized out of order there could be a situation where
// an edge with the label r is deserialized before the edges with the lables r-x...r-1. This function creates, if needed, all lables matrix required until reaching r  - used for graph deserialization.
static inline void _Graph_TryAddRelationMatrix(Graph *g, int r) {
	if(r == GRAPH_NO_RELATION) return;
	uint relation_count = Graph_RelationTypeCount(g);
	if(r >= relation_count) {
		for(uint i = relation_count; i <= r; i++) Graph_AddRelationType(g);
	}
}

// Set a given edge in the graph - Used for deserialization of graph.
void Graph_SetEdge(Graph *g, EdgeID edge_id, NodeID src, NodeID dest, int r, Edge *e) {
	GrB_Info info;

	Entity *en = DataBlock_AllocateItemOutOfOrder(g->edges, edge_id);
	en->id = edge_id;
	en->prop_count = 0;
	en->properties = NULL;
	e->entity = en;
	e->relationID = r;
	e->srcNodeID = src;
	e->destNodeID = dest;

	_Graph_TryAddRelationMatrix(g, r);
	GrB_Matrix adj = Graph_GetAdjacencyMatrix(g);
	GrB_Matrix relationMat = Graph_GetRelationMatrix(g, r);
	GrB_Matrix tadj = Graph_Get_Transposed_AdjacencyMatrix(g);
	// Rows represent source nodes, columns represent destination nodes.
	GrB_Matrix_setElement_BOOL(adj, true, src, dest);
	GrB_Matrix_setElement_BOOL(tadj, true, dest, src);
	GrB_Index I = src;
	GrB_Index J = dest;
	edge_id = SET_MSB(edge_id);
	info = GxB_Matrix_subassign_UINT64   // C(I,J)<Mask> = accum (C(I,J),x)
		   (
			   relationMat,         // input/output matrix for results
			   GrB_NULL,            // optional mask for C(I,J), unused if NULL
			   graph_edge_accum,   // optional accum for Z=accum(C(I,J),x)
			   edge_id,             // scalar to assign to C(I,J)
			   &I,                  // row indices
			   1,                   // number of row indices
			   &J,                  // column indices
			   1,                   // number of column indices
			   GrB_NULL             // descriptor for C(I,J) and Mask
		   );
	assert(info == GrB_SUCCESS);
}

inline void Graph_MarkEdgeDeleted(Graph *g, EdgeID id) {
	assert(g);
	DataBlock_MarkAsDeletedOutOfOrder(g->edges, id);
}

inline void Graph_MarkNodeDeleted(Graph *g, NodeID id) {
	assert(g);
	DataBlock_MarkAsDeletedOutOfOrder(g->nodes, id);
}

inline void Graph_TryAddLabelMatrix(Graph *g, int label) {
	if(label == GRAPH_NO_LABEL) return;
	int label_count = Graph_LabelTypeCount(g);
	if(label >= label_count) {
		for(int i = label_count; i <= label; i++) Graph_AddLabel(g);
	}
}

