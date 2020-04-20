/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v7.h"
#include <assert.h>
#include "../../encoder_common.h"

extern uint64_t entities_threshold;

static void _RdbSaveEntity(RedisModuleIO *rdb, const Entity *e) {
	/* Format:
	 * #attributes N
	 * (name, value type, value) X N  */

	RedisModule_SaveUnsigned(rdb, e->prop_count);

	for(int i = 0; i < e->prop_count; i++) {
		EntityProperty attr = e->properties[i];
		RedisModule_SaveUnsigned(rdb, attr.id);
		RdbSaveSIValue(rdb, &attr.value);
	}
}

static void _RdbSaveEdge(RedisModuleIO *rdb, const Graph *g, const Edge *e, int r) {

	/* Format:
	* edge
	* {
	*  edge ID
	*  source node ID
	*  destination node ID
	*  relation type
	* }
	* edge properties */

	RedisModule_SaveUnsigned(rdb, ENTITY_GET_ID(e));

	// Source node ID.
	RedisModule_SaveUnsigned(rdb, Edge_GetSrcNodeID(e));

	// Destination node ID.
	RedisModule_SaveUnsigned(rdb, Edge_GetDestNodeID(e));

	// Relation type.
	RedisModule_SaveUnsigned(rdb, r);

	// Edge properties.
	_RdbSaveEntity(rdb, e->entity);
}

// Update the next encoding phase if needed.
static void _UpdateEncodePhase(GraphContext *gc) {
	EncodePhase current_phase = GraphEncodeContext_GetEncodePhase(gc->encoding_context);
	switch(current_phase) {
	case NODES:
		// Check if NODES encodeding phase is done
		if(GraphEncodeContext_GetProcessedNodesCount(gc->encoding_context) < Graph_NodeCount(gc->g)) return;
		// We are done with nodes, set phase to DELETED_NODES and fall though to its case
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, DELETED_NODES);
	case DELETED_NODES:
		// Check if there is a need to encoded deleted nodes, or skip to edges.
		if(array_len(gc->g->nodes->deletedIdx) >
		   GraphEncodeContext_GetProcessedDeletedNodesCount(gc->encoding_context)) return;
		// No deleted nodes left. Set state to EDGES and fall though to its case.
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, EDGES);
	case EDGES:
		// Check if need to encode edges, deleted edges or skip tp schema.
		if(GraphEncodeContext_GetProcessedEdgesCount(gc->encoding_context) < Graph_EdgeCount(gc->g)) return;
		// We are done with edges, set phase to DELETED_EDGES and fall though to its case
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, DELETED_EDGES);
	case DELETED_EDGES:
		// Check if there is a need to encoded deleted edges, or skip to schema.
		if(array_len(gc->g->edges->deletedIdx) >
		   GraphEncodeContext_GetProcessedDeletedEdgesCount(gc->encoding_context))return;
		// No deleted edges left. Set state to GRAPH_SCHEMA and fall though to its case.
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, GRAPH_SCHEMA);
		return;
	default:
		assert(false && "Unkown encode phase");
	}
}

void RdbSaveDeletedNodes_v7(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #deleted nodes N
	 * node id X N */

	// Get graph's deleted node count.
	uint64_t deleted_nodes = array_len(gc->g->nodes->deletedIdx);
	// Get the number of deleted nodes already encoded.
	uint64_t encoded_deleted_nodes = GraphEncodeContext_GetProcessedDeletedNodesCount(
										 gc->encoding_context);
	// Calculate the number of deleted nodes required to encode in this phase.
	uint64_t deleted_nodes_to_encode = deleted_nodes - encoded_deleted_nodes;
	// If the required number is bigger than the allowed entities threshold, set it to be the threshold.
	if(deleted_nodes_to_encode > entities_threshold) deleted_nodes_to_encode = entities_threshold;

	// # Deleted nodes
	RedisModule_SaveUnsigned(rdb, deleted_nodes_to_encode);
	// Iterated over the required range in the datablock deleted items.
	for(uint64_t i = encoded_deleted_nodes; i < encoded_deleted_nodes + deleted_nodes_to_encode; i++) {
		RedisModule_SaveUnsigned(rdb, gc->g->nodes->deletedIdx[i]);
	}
	GraphEncodeContext_SetProcessedDeletedNodesCount(gc->encoding_context,
													 encoded_deleted_nodes + deleted_nodes_to_encode);
	_UpdateEncodePhase(gc);
}

void RdbSaveNodes_v7(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #nodes
	 *      ID
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N */

	// Get graph's node count.
	uint64_t graph_nodes = Graph_NodeCount(gc->g);
	// Get the number of nodes already encoded.
	uint64_t encoded_nodes = GraphEncodeContext_GetProcessedNodesCount(gc->encoding_context);
	// Calculate the number of nodes required to encode in this phase.
	uint64_t nodes_to_encode = graph_nodes - encoded_nodes;
	// If the required number is bigger than the allowed entities threshold, set it to be the threshold.
	if(nodes_to_encode > entities_threshold) nodes_to_encode = entities_threshold;
	// Get datablock iterator from context, or create new one.
	DataBlockIterator *iter = GraphEncodeContext_GetDatablockIterator(gc->encoding_context);
	if(!iter) iter = Graph_ScanNodes(gc->g);
	// #Nodes
	RedisModule_SaveUnsigned(rdb, nodes_to_encode);
	for(uint64_t i = 0; i < nodes_to_encode; i++) {
		Entity *e = (Entity *)DataBlockIterator_Next(iter);
		// Save ID
		RedisModule_SaveUnsigned(rdb, e->id);
		int l = Graph_GetNodeLabel(gc->g, e->id);

		// #labels, currently only one label per node.
		int label_count = (l == GRAPH_NO_LABEL) ? 0 : 1;
		RedisModule_SaveUnsigned(rdb, label_count);

		// (label)
		if(label_count) RedisModule_SaveUnsigned(rdb, l);

		// properties N
		// (name, value type, value) X N
		_RdbSaveEntity(rdb, e);

	}

	// Check if done encodeing nodes.
	if(encoded_nodes + nodes_to_encode == graph_nodes) {
		DataBlockIterator_Free(iter);
		iter = NULL;
	}

	// Update context.
	GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	GraphEncodeContext_SetProcessedNodesCount(gc->encoding_context, encoded_nodes + nodes_to_encode);
	_UpdateEncodePhase(gc);
}

void RdbSaveDeletedEdges_v7(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #deleted edges N
	 * edge id X N */

	// Get graph's deleted edge count.
	uint64_t deleted_edges = array_len(gc->g->edges->deletedIdx);
	// Get the number of deleted edges already encoded.
	uint64_t encoded_deleted_edges = GraphEncodeContext_GetProcessedDeletedEdgesCount(
										 gc->encoding_context);
	// Calculate the number of deleted edges required to encode in this phase.
	uint64_t deleted_edges_to_encode = deleted_edges - encoded_deleted_edges;
	// If the required number is bigger than the allowed entities threshold, set it to be the threshold.
	if(deleted_edges_to_encode > entities_threshold) deleted_edges_to_encode = entities_threshold;

	// # Deleted edges
	RedisModule_SaveUnsigned(rdb, deleted_edges_to_encode);
	// Iterated over the required range in the datablock deleted items.
	for(uint64_t i = encoded_deleted_edges; i < encoded_deleted_edges + deleted_edges_to_encode; i++) {
		RedisModule_SaveUnsigned(rdb, gc->g->edges->deletedIdx[i]);
	}
	GraphEncodeContext_SetProcessedDeletedEdgesCount(gc->encoding_context,
													 encoded_deleted_edges + deleted_edges_to_encode);
	_UpdateEncodePhase(gc);
}

/* Auxilary function to encode a multiple edges array, while consdirating the allowed number of edges to encode. Returns true if the number of encoded edges
 * has reached the capacity. */
static bool _RdbSaveMultipleEdges(RedisModuleIO *rdb,                  // RDB IO.
								  GraphContext *gc,                    // Graph context.
								  uint r,                              // Edges relation id.
								  EdgeID *multiple_edges_array,        // Multiple edges array (passed by ref).
								  uint *multiple_edges_current_index,  // Current index of the array to start encoding from (passed by ref).
								  uint64_t *encoded_edges,             // Number of encoded edges in this phase (passed by ref).
								  uint64_t edges_to_encode,            // Allowed capacity for encoding edges.
								  NodeID src,                          // Edges source node id.
								  NodeID dest                          // Edges destination node id.
								 ) {
	uint edgeCount = array_len(multiple_edges_array);
	// Add edges as long the number of encoded edges is in the allowed range, and the array is not depleted.
	while(*multiple_edges_current_index < edgeCount && *encoded_edges < edges_to_encode) {
		Edge e;
		EdgeID edgeID = multiple_edges_array[(*multiple_edges_current_index)++];
		e.srcNodeID = src;
		e.destNodeID = dest;
		Graph_GetEdge(gc->g, edgeID, &e);
		_RdbSaveEdge(rdb, gc->g, &e, r);
		(*encoded_edges)++;
	}
	// Return true if the number of encoded edges is equal to the edges capacity
	return *encoded_edges == edges_to_encode;
}

void RdbSaveEdges_v7(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #edges (N)
	 * {
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 * } X N
	 * edge properties X N */

	// Get graph's edge count.
	uint64_t graph_edges = Graph_EdgeCount(gc->g);
	// Get the number of edges already encoded.
	uint64_t currently_encoded_edges = GraphEncodeContext_GetProcessedEdgesCount(gc->encoding_context);
	// Calculate the number of edges required to encode in this phase.
	uint64_t edges_to_encode = graph_edges - currently_encoded_edges;
	// If the required number is bigger than the allowed entities threshold, set it to be the threshold.
	if(edges_to_encode > entities_threshold) edges_to_encode = entities_threshold;

	// Count the edges that will be encoded in this phase.
	uint64_t encoded_edges = 0;

	// Get current relation matrix.
	uint r = GraphEncodeContext_GetCurrentRelationID(gc->encoding_context);

	GrB_Matrix M = Graph_GetRelationMatrix(gc->g, r);
	// Get matrix tuple iterator from context, or create new one.
	GxB_MatrixTupleIter *iter = GraphEncodeContext_GetMatrixTupleIterator(gc->encoding_context);
	if(!iter) GxB_MatrixTupleIter_new(&iter, M);

	// First, see if the last edges encoding stopped at multiple edges array
	EdgeID *multiple_edges_array = GraphEncodeContext_GetMultipleEdgesArray(gc->encoding_context);
	uint multiple_edges_current_index = GraphEncodeContext_GetMultipleEdgesCurrentIndex(
											gc->encoding_context);
	NodeID src = GraphEncodeContext_GetMultipleEdgesSourceNode(gc->encoding_context);
	NodeID dest = GraphEncodeContext_GetMultipleEdgesDestinationNode(gc->encoding_context);
	// #Edges
	RedisModule_SaveUnsigned(rdb, edges_to_encode);
	if(multiple_edges_array) {
		bool finished = _RdbSaveMultipleEdges(rdb, gc, r, multiple_edges_array,
											  &multiple_edges_current_index,
											  &encoded_edges, edges_to_encode, src, dest);
		if(finished) { // If the multiple edges array filled the capacity of entities allowed to be encoded, finish encoding.
			goto finish;
		} else {
			// Reset the multiple edges context for re-use.
			multiple_edges_array = NULL;
			multiple_edges_current_index = 0;
		}
	}
	uint relation_count = Graph_RelationTypeCount(gc->g);
	while(encoded_edges < edges_to_encode) {
		Edge e;
		EdgeID edgeID;
		bool depleted = false;
		// Try to get next tuple.
		GxB_MatrixTupleIter_next(iter, &src, &dest, &depleted);
		// If iterator is depleted
		while(depleted && r < relation_count) {
			// Free iterator
			GxB_MatrixTupleIter_free(iter);
			iter = NULL;
			depleted = false;
			// Proceed to next relation matrix.
			r++;
			// If done iterating over all the matrices, jump to finish.
			if(r == relation_count) goto finish;
			// Get matrix and set iterator.
			M = Graph_GetRelationMatrix(gc->g, r);
			GxB_MatrixTupleIter_new(&iter, M);
			GxB_MatrixTupleIter_next(iter, &src, &dest, &depleted);
		}

		e.srcNodeID = src;
		e.destNodeID = dest;
		GrB_Matrix_extractElement_UINT64(&edgeID, M, e.srcNodeID, e.destNodeID);
		if(SINGLE_EDGE(edgeID)) {
			edgeID = SINGLE_EDGE_ID(edgeID);
			Graph_GetEdge(gc->g, edgeID, &e);
			_RdbSaveEdge(rdb, gc->g, &e, r);
			encoded_edges++;
		} else {
			multiple_edges_array = (EdgeID *)edgeID;
			bool finished = _RdbSaveMultipleEdges(rdb, gc, r, multiple_edges_array,
												  &multiple_edges_current_index,
												  &encoded_edges, edges_to_encode, src, dest);
			if(finished) { // If the multiple edges array filled the capacity of entities allowed to be encoded, finish encoding.
				goto finish;
			} else {
				// Reset the multiple edges context for re-use.
				multiple_edges_array = NULL;
				multiple_edges_current_index = 0;
			}
		}
	}

finish:
	// Check if done encoding edges.
	if(currently_encoded_edges + edges_to_encode == graph_edges) {
		if(iter) {
			GxB_MatrixTupleIter_free(iter);
			iter = NULL;
		}
	}

	// Update context.
	GraphEncodeContext_SetCurrentRelationID(gc->encoding_context, r);
	GraphEncodeContext_SetMatrixTupleIterator(gc->encoding_context, iter);
	GraphEncodeContext_SetProcessedEdgesCount(gc->encoding_context,
											  currently_encoded_edges + edges_to_encode);
	GraphEncodeContext_SetMutipleEdgesArray(gc->encoding_context, multiple_edges_array,
											multiple_edges_current_index, src, dest);
	_UpdateEncodePhase(gc);
}
