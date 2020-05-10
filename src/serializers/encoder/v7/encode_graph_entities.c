/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v7.h"
#include <assert.h>

// Forword decleration.
static void _RdbSaveSIValue(RedisModuleIO *rdb, const SIValue *v);

static void _RdbSaveSIArray(RedisModuleIO *rdb, const SIValue list) {
	/* saves array as
	   unsigned : array legnth
	   array[0]
	   .
	   .
	   .
	   array[array length -1]
	 */
	uint arrayLen = SIArray_Length(list);
	RedisModule_SaveUnsigned(rdb, arrayLen);
	for(uint i = 0; i < arrayLen; i ++) {
		SIValue value = SIArray_Get(list, i);
		_RdbSaveSIValue(rdb, &value);
	}
}

static void _RdbSaveSIValue(RedisModuleIO *rdb, const SIValue *v) {
	/* Format:
	 * SIType
	 * Value */
	RedisModule_SaveUnsigned(rdb, v->type);
	switch(v->type) {
	case T_BOOL:
	case T_INT64:
		RedisModule_SaveSigned(rdb, v->longval);
		return;
	case T_DOUBLE:
		RedisModule_SaveDouble(rdb, v->doubleval);
		return;
	case T_STRING:
		RedisModule_SaveStringBuffer(rdb, v->stringval, strlen(v->stringval) + 1);
		return;
	case T_ARRAY:
		_RdbSaveSIArray(rdb, *v);
		return;
	case T_NULL:
		return; // No data beyond the type needs to be encoded for a NULL value.
	default:
		assert(0 && "Attempted to serialize value of invalid type.");
	}
}

static void _RdbSaveEntity(RedisModuleIO *rdb, const Entity *e) {
	/* Format:
	 * #attributes N
	 * (name, value type, value) X N  */

	RedisModule_SaveUnsigned(rdb, e->prop_count);

	for(int i = 0; i < e->prop_count; i++) {
		EntityProperty attr = e->properties[i];
		RedisModule_SaveUnsigned(rdb, attr.id);
		_RdbSaveSIValue(rdb, &attr.value);
	}
}

static void _RdbSaveEdge(RedisModuleIO *rdb, const Graph *g, const Edge *e, int r) {

	/* Format:
	 *  edge ID
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 *  edge properties */

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

static void _RdbSaveNode_v7(RedisModuleIO *rdb, GraphContext *gc, Entity *e) {
	/* Format:
	*      ID
	*      #labels M
	*      (labels) X M
	*      #properties N
	*      (name, value type, value) X N */

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

static void _RdbSaveDeletedEntities_v7(RedisModuleIO *rdb, GraphContext *gc,
									   uint64_t deleted_entities_to_encode, uint64_t *deleted_id_list) {
	// Get deleted entities count.
	uint64_t deleted_entities = array_len(deleted_id_list);
	// Get the number of deleted entities already encoded.
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// Iterated over the required range in the datablock deleted items.
	for(uint64_t i = offset; i < offset + deleted_entities_to_encode; i++) {
		RedisModule_SaveUnsigned(rdb, deleted_id_list[i]);
	}
}

void RdbSaveDeletedNodes_v7(RedisModuleIO *rdb, GraphContext *gc,
							uint64_t deleted_nodes_to_encode) {
	/* Format:
	 * node id X N */

	if(deleted_nodes_to_encode == 0) return;
	// Get deleted nodes list.
	uint64_t *deleted_nodes_list = Serializer_Graph_GetDeletedNodesList(gc->g);
	_RdbSaveDeletedEntities_v7(rdb, gc, deleted_nodes_to_encode, deleted_nodes_list);
}

void RdbSaveDeletedEdges_v7(RedisModuleIO *rdb, GraphContext *gc,
							uint64_t deleted_edges_to_encode) {
	/* Format:
	 * edge id X N */

	if(deleted_edges_to_encode == 0) return;
	// Get deleted edges list.
	uint64_t *deleted_edges_list = Serializer_Graph_GetDeletedEdgesList(gc->g);
	_RdbSaveDeletedEntities_v7(rdb, gc, deleted_edges_to_encode, deleted_edges_list);
}

void RdbSaveNodes_v7(RedisModuleIO *rdb, GraphContext *gc, uint64_t nodes_to_encode) {
	/* Format:
	 * Node Format * nodes_to_encode:
	 *  ID
	 *  #labels M
	 *  (labels) X M
	 *  #properties N
	 *  (name, value type, value) X N
	 */

	if(nodes_to_encode == 0) return;
	// Get graph's node count.
	uint64_t graph_nodes = Graph_NodeCount(gc->g);
	// Get the number of nodes already encoded.
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// Get datablock iterator from context, already set to offset by a previous encodeing of nodes, or create new one.
	DataBlockIterator *iter = GraphEncodeContext_GetDatablockIterator(gc->encoding_context);
	if(!iter) {
		iter = Graph_ScanNodes(gc->g);
		GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	}

	for(uint64_t i = 0; i < nodes_to_encode; i++) {
		Entity *e = (Entity *)DataBlockIterator_Next(iter);
		_RdbSaveNode_v7(rdb, gc, e);
	}

	// Check if done encodeing nodes.
	if(offset + nodes_to_encode == graph_nodes) {
		DataBlockIterator_Free(iter);
		iter = NULL;
		GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	}
}

/* Auxilary function to encode a multiple edges array, while consdirating the allowed number of edges to encode. Returns true if the number of encoded edges
 * has reached the capacity. */
static void _RdbSaveMultipleEdges(RedisModuleIO *rdb,                  // RDB IO.
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
	// Define function local variables from passed-by-reference parameters.
	uint i = *multiple_edges_current_index;
	uint encoded_edges_count = *encoded_edges;
	// Add edges as long the number of encoded edges is in the allowed range, and the array is not depleted.
	while(i < edgeCount && encoded_edges_count < edges_to_encode) {
		Edge e;
		EdgeID edgeID = multiple_edges_array[i++];
		e.srcNodeID = src;
		e.destNodeID = dest;
		Graph_GetEdge(gc->g, edgeID, &e);
		_RdbSaveEdge(rdb, gc->g, &e, r);
		encoded_edges_count++;
	}
	// Update passed-by-reference parameters.
	*encoded_edges = encoded_edges_count;
	*multiple_edges_current_index = i;
}

void RdbSaveEdges_v7(RedisModuleIO *rdb, GraphContext *gc, uint64_t edges_to_encode) {
	/* Format:
	 * Edge format * edges_to_encode:
	 *  edge ID
	 *  source node ID
	 *  destination node ID
	 *  relation type
	 *  edge properties
	 * */

	if(edges_to_encode == 0) return;
	// Get graph's edge count.
	uint64_t graph_edges = Graph_EdgeCount(gc->g);
	// Get the number of edges already encoded.
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// Count the edges that will be encoded in this phase.
	uint64_t encoded_edges = 0;

	// Get current relation matrix.
	uint r = GraphEncodeContext_GetCurrentRelationID(gc->encoding_context);

	GrB_Matrix M = Graph_GetRelationMatrix(gc->g, r);
	// Get matrix tuple iterator from context, already set to the next entry to fetch, for previous edge encide or create new one.
	GxB_MatrixTupleIter *iter = GraphEncodeContext_GetMatrixTupleIterator(gc->encoding_context);
	if(!iter) GxB_MatrixTupleIter_new(&iter, M);

	// First, see if the last edges encoding stopped at multiple edges array
	EdgeID *multiple_edges_array = GraphEncodeContext_GetMultipleEdgesArray(gc->encoding_context);
	NodeID src = GraphEncodeContext_GetMultipleEdgesSourceNode(gc->encoding_context);;
	NodeID dest = GraphEncodeContext_GetMultipleEdgesDestinationNode(gc->encoding_context);;
	uint multiple_edges_current_index = GraphEncodeContext_GetMultipleEdgesCurrentIndex(
											gc->encoding_context);
	if(multiple_edges_array) {
		_RdbSaveMultipleEdges(rdb, gc, r, multiple_edges_array,
							  &multiple_edges_current_index,
							  &encoded_edges, edges_to_encode, src, dest);
		// If the multiple edges array filled the capacity of entities allowed to be encoded, finish encoding.
		if(encoded_edges == edges_to_encode) {
			goto finish;
		} else {
			// Reset the multiple edges context for re-use.
			multiple_edges_array = NULL;
			multiple_edges_current_index = 0;
		}
	}

	uint relation_count = Graph_RelationTypeCount(gc->g);
	// Write the required number of edges.
	while(encoded_edges < edges_to_encode) {
		Edge e;
		EdgeID edgeID;
		bool depleted = false;
		// Try to get next tuple.
		GxB_MatrixTupleIter_next(iter, &src, &dest, &depleted);
		// If iterator is depleted, get new tuple from different matrix or finish encode.
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
			_RdbSaveMultipleEdges(rdb, gc, r, multiple_edges_array,
								  &multiple_edges_current_index, &encoded_edges, edges_to_encode, src, dest);
			// If the multiple edges array filled the capacity of entities allowed to be encoded, finish encoding.
			if(encoded_edges == edges_to_encode) {
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
	if(offset + edges_to_encode == graph_edges) {
		if(iter) {
			GxB_MatrixTupleIter_free(iter);
			iter = NULL;
		}
	}

	// Update context.
	GraphEncodeContext_SetCurrentRelationID(gc->encoding_context, r);
	GraphEncodeContext_SetMatrixTupleIterator(gc->encoding_context, iter);
	GraphEncodeContext_SetMutipleEdgesArray(gc->encoding_context, multiple_edges_array,
											multiple_edges_current_index, src, dest);
}
