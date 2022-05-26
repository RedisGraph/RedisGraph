/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "graph.h"
#include "../configuration/config.h"
#include "../util/datablock/oo_datablock.h"

// clones a graph object
Graph *Graph_Clone
(
	const Graph *g  // graph to clone
) {
	// the following elements will be clones
	// 1. nodes
	// 2. edges
	// 3. matrices
	
	// create an empty graph
	size_t node_cap;
	size_t edge_cap;
	assert(Config_Option_get(Config_NODE_CREATION_BUFFER, &node_cap));
	assert(Config_Option_get(Config_NODE_CREATION_BUFFER, &edge_cap));
	Graph *clone = Graph_New(node_cap, edge_cap);
	ASSERT(clone != NULL);

	size_t node_count = Graph_NodeCount(g) + Graph_DeletedNodeCount(g);
	size_t edge_count = Graph_EdgeCount(g) + Graph_DeletedEdgeCount(g);
	Graph_AllocateNodes(clone, node_count);
	Graph_AllocateEdges(clone, edge_count);

	//--------------------------------------------------------------------------
	// clone graph entities
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// clone nodes
	//--------------------------------------------------------------------------

	EntityID node_id;    // ID of node being cloned
	Entity *node;        // node to clone
	Entity *node_clone;  // cloned node

	DataBlockIterator *it = DataBlock_Scan(g->nodes);  // node iterator

	// clone each node in the graph
	while((node = DataBlockIterator_Next(it, &node_id)) != NULL) {
		// maintain node ID
		node_clone = DataBlock_AllocateItemOutOfOrder(clone->nodes, node_id);
		GraphEntity_Clone(node, node_clone);
	}

	DataBlockIterator_Free(it);

	// clone deleted nodes
	uint deleted_node_count = DataBlock_DeletedItemsCount(g->nodes);
	uint64_t *deleted_nodes_ids = g->nodes->deletedIdx;
	for(uint i = 0; i < deleted_node_count; i++) {
		EntityID deleted_node_id = deleted_nodes_ids[i];
		DataBlock_MarkAsDeletedOutOfOrder(clone->nodes, deleted_node_id);
	}

	//--------------------------------------------------------------------------
	// clone edges
	//--------------------------------------------------------------------------

	EntityID edge_id;    // ID of edge being cloned
	Entity *edge;        // edge to clone
	Entity *edge_clone;  // cloned edge

	it = DataBlock_Scan(g->edges);  // edge iterator

	// clone each edge in the graph
	while((edge = DataBlockIterator_Next(it, &edge_id)) != NULL) {
		// maintain edge ID
		edge_clone = DataBlock_AllocateItemOutOfOrder(clone->edges, edge_id);
		GraphEntity_Clone(edge, edge_clone);
	}

	DataBlockIterator_Free(it);

	// clone deleted edges
	uint deleted_edge_count = DataBlock_DeletedItemsCount(g->edges);
	uint64_t *deleted_edges_ids = g->edges->deletedIdx;
	for(uint i = 0; i < deleted_edge_count; i++) {
		EntityID deleted_edge_id = deleted_edges_ids[i];
		DataBlock_MarkAsDeletedOutOfOrder(clone->edges, deleted_edge_id);
	}

	//--------------------------------------------------------------------------
	// clone graph matrices
	//--------------------------------------------------------------------------

	//--------------------------------------------------------------------------
	// clone ADJ matrix
	//--------------------------------------------------------------------------

	RG_Matrix ADJ_clone;
	RG_Matrix ADJ = Graph_GetAdjacencyMatrix(g, false);
	RG_Matrix_dup(&ADJ_clone, ADJ);

	// assign clone's ADJ matrix
	RG_Matrix_free(&clone->adjacency_matrix);
	clone->adjacency_matrix = ADJ_clone;

	//--------------------------------------------------------------------------
	// clone node labels matrix
	//--------------------------------------------------------------------------

	RG_Matrix node_labels_clone;
	RG_Matrix node_labels = Graph_GetNodeLabelMatrix(g);
	RG_Matrix_dup(&node_labels_clone, node_labels);

	// assign clone's node labels matrix
	RG_Matrix_free(&clone->node_labels);
	clone->node_labels = node_labels_clone;

	//--------------------------------------------------------------------------
	// clone label matrices
	//--------------------------------------------------------------------------

	int label_count = Graph_LabelTypeCount(g);
	for(int i = 0; i < label_count; i++) {
		Graph_AddLabel(clone);
		RG_Matrix L_clone;
		RG_Matrix L = Graph_GetLabelMatrix(g, i);
		RG_Matrix_dup(&L_clone, L);

		// assign clone's label matrix
		RG_Matrix_free(clone->labels+i);
		clone->labels[i] = L_clone;
	}

	//--------------------------------------------------------------------------
	// clone relation matrices
	//--------------------------------------------------------------------------

	int relation_count = Graph_RelationTypeCount(g);
	for(int i = 0; i < relation_count; i++) {
		Graph_AddRelationType(clone);
		RG_Matrix R_clone;
		RG_Matrix R = Graph_GetRelationMatrix(g, i, false);
		if(Graph_RelationshipContainsMultiEdge(g, i, false)) {
			// TODO: clone multi-edge matrix	
		} else {
			RG_Matrix_dup(&R_clone, R);
		}

		// assign clone's relationship matrix
		RG_Matrix_free(clone->relations+i);
		clone->relations[i] = R_clone;
	}

	//--------------------------------------------------------------------------
	// clone graph statistics
	//--------------------------------------------------------------------------

	clone->stats = GraphStatistics_Clone(&g->stats);

	return clone;
}

