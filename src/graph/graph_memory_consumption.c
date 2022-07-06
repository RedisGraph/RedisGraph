/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphcontext.h"

// estimate graph entity memory consumption
static size_t _GraphEntity_memoryUsage
(
	const GraphEntity *ge
) {
	size_t n = sizeof(GraphEntity);
	const AttributeSet set = GraphEntity_GetAttributes(ge);
	int attr_count = ATTRIBUTE_SET_COUNT(set);

	for(int i = 0; i < attr_count; i++) {
		Attribute_ID attr_id;
		SIValue val = AttributeSet_GetIdx(set, i, &attr_id);
		n += SIValue_memoryUsage(val);
	}

	return n;
}

// estimate graph's memory usage
size_t GraphContext_MemoryUsage
(
	const GraphContext *gc
) {
	ASSERT(gc != NULL);

	// graph's memory usage is largly effected by the following components
	// 1. nodes
	// 2. edges
	// 3. matrices
	// 4. indicies
	// 5. datablocks
	
	Graph *g = gc->g;
	
	GrB_Info info;

	RG_Matrix A    = NULL;
	size_t    n    = 0;
	size_t    size = 0;

	uint      node_count        = Graph_NodeCount(g);
	uint      edge_count        = Graph_EdgeCount(g);
	uint      matrix_dim        = Graph_RequiredMatrixDim(g);
	int       label_count       = Graph_LabelTypeCount(g);
	int       relation_count    = Graph_RelationTypeCount(g);

	EntityID max_node_id       = node_count + Graph_DeletedNodeCount(g);
	EntityID max_edge_id       = edge_count + Graph_DeletedEdgeCount(g);
	size_t   nodes_memoryUsage = 0;
	size_t   edges_memoryUsage = 0;
	int      sample_count      = 0;

	//--------------------------------------------------------------------------
	// account for label matrices
	//--------------------------------------------------------------------------

	for(int i = 0; i < label_count; i++) {
		A = Graph_GetLabelMatrix(g, i);
		info = RG_Matrix_memoryUsage(&size, A);
		ASSERT(info == GrB_SUCCESS);
		n += size;
	}

	//--------------------------------------------------------------------------
	// account for relation matrices (including transposes)
	//--------------------------------------------------------------------------

	for(int i = 0; i < relation_count; i++) {
		A = Graph_GetRelationMatrix(g, i, false);
		info = RG_Matrix_memoryUsage(&size, A);
		ASSERT(info == GrB_SUCCESS);
		n += size;
	}

	//--------------------------------------------------------------------------
	// ADJ matrix
	//--------------------------------------------------------------------------

	A = Graph_GetAdjacencyMatrix(g, false);
	info = RG_Matrix_memoryUsage(&size, A);
	ASSERT(info == GrB_SUCCESS);
	n += size;

	//--------------------------------------------------------------------------
	// labels matrix
	//--------------------------------------------------------------------------

	A = Graph_GetNodeLabelMatrix(g);
	info = RG_Matrix_memoryUsage(&size, A);
	ASSERT(info == GrB_SUCCESS);
	n += size;

	//--------------------------------------------------------------------------
	// nodes estimated memory consumption
	//--------------------------------------------------------------------------

	if(node_count > 0) {
		// randomly inspect MIN(%10, 10000) of the nodes
		sample_count = MIN(10000, 0.1 * node_count);
		for(int i = 0; i < sample_count; i++) {
			// pick a random node
			Node ge;
			EntityID id = rand() % max_node_id;
			Graph_GetNode(g, id, &ge);

			nodes_memoryUsage += _GraphEntity_memoryUsage((GraphEntity*)&ge);
		}

		// generalize for the entire node set
		size_t avg_nodeMemoryUsage = (nodes_memoryUsage / sample_count);
		n += avg_nodeMemoryUsage * node_count;
	}

	//--------------------------------------------------------------------------
	// edges estimated memory consumption
	//--------------------------------------------------------------------------
	
	if(edge_count > 0) {
		// randomly inspect MIN(%10, 10000) of the edges
		sample_count = MIN(10000, 0.1 * edge_count);
		for(int i = 0; i < sample_count; i++) {
			// pick a random edge
			Edge ge;
			EntityID id = rand() % max_edge_id;
			Graph_GetEdge(g, id, &ge);

			edges_memoryUsage += _GraphEntity_memoryUsage((GraphEntity*)&ge);
		}

		// generalize for the entire edge set
		size_t avg_edgeMemoryUsage = (edges_memoryUsage / sample_count);
		n += avg_edgeMemoryUsage * edge_count;
	}

	//--------------------------------------------------------------------------
	// datablocks
	//--------------------------------------------------------------------------
	
	n += DataBlock_memoryUsage(g->nodes);
	n += DataBlock_memoryUsage(g->edges);

	// TODO: estimated indicies memory consumption

	return n;
}

