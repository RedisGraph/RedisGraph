/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v12.h"
#include "../../../datatypes/datatypes.h"

// forword decleration
static void _RdbSaveSIValue
(
	RedisModuleIO *rdb,
	const SIValue *v
);

static void _RdbSaveSIArray
(
	RedisModuleIO *rdb,
	const SIValue list
) {
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

static void _RdbSaveSIValue
(
	RedisModuleIO *rdb,
	const SIValue *v
) {
	// Format:
	// SIType
	// Value
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
		case T_POINT:
			RedisModule_SaveDouble(rdb, Point_lat(*v));
			RedisModule_SaveDouble(rdb, Point_lon(*v));
		case T_NULL:
			return; // No data beyond the type needs to be encoded for a NULL value.
		default:
			ASSERT(0 && "Attempted to serialize value of invalid type.");
	}
}

static void _RdbSaveEntity
(
	RedisModuleIO *rdb,
	const GraphEntity *e
) {
	// Format:
	// #attributes N
	// (name, value type, value) X N 

	const AttributeSet set = GraphEntity_GetAttributes(e);

	RedisModule_SaveUnsigned(rdb, ATTRIBUTE_SET_COUNT(set));

	for(int i = 0; i < ATTRIBUTE_SET_COUNT(set); i++) {
		Attribute_ID attr_id;
		SIValue value = AttributeSet_GetIdx(set, i, &attr_id);
		RedisModule_SaveUnsigned(rdb, attr_id);
		_RdbSaveSIValue(rdb, &value);
	}
}

static void _RdbSaveEdge_v12
(
	RedisModuleIO *rdb,
	const GraphEntity *e
) {

	// Format:
	//  edge ID
	//  source node ID
	//  destination node ID
	//  relation type
	//  edge properties

	RedisModule_SaveUnsigned(rdb, ENTITY_GET_ID(e));

	// edge properties
	_RdbSaveEntity(rdb, (GraphEntity *)e);
}

static void _RdbSaveNode_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	GraphEntity *n
) {
	// Format:
	//     ID
	//     #labels M
	//     (labels) X M
	//     #properties N
	//     (name, value type, value) X N */

	// save ID
	EntityID id = ENTITY_GET_ID(n);
	RedisModule_SaveUnsigned(rdb, id);

	// retrieve node labels
	uint l_count;
	NODE_GET_LABELS(gc->g, (Node *)n, l_count);
	RedisModule_SaveUnsigned(rdb, l_count);

	// save labels
	for(uint i = 0; i < l_count; i++) RedisModule_SaveUnsigned(rdb, labels[i]);

	// properties N
	// (name, value type, value) X N
	_RdbSaveEntity(rdb, (GraphEntity *)n);
}

static void _RdbSaveDeletedEntities_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_entities_to_encode,
	uint64_t *deleted_id_list
) {
	// Get the number of deleted entities already encoded.
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// Iterated over the required range in the datablock deleted items.
	for(uint64_t i = offset; i < offset + deleted_entities_to_encode; i++) {
		RedisModule_SaveUnsigned(rdb, deleted_id_list[i]);
	}
}

void RdbSaveDeletedNodes_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_nodes_to_encode
) {
	// Format:
	// node id X N

	if(deleted_nodes_to_encode == 0) return;
	// get deleted nodes list
	uint64_t *deleted_nodes_list = Serializer_Graph_GetDeletedNodesList(gc->g);
	_RdbSaveDeletedEntities_v12(rdb, gc, deleted_nodes_to_encode, deleted_nodes_list);
}

void RdbSaveDeletedEdges_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_edges_to_encode
) {
	// Format:
	// edge id X N

	if(deleted_edges_to_encode == 0) return;

	// get deleted edges list
	uint64_t *deleted_edges_list = Serializer_Graph_GetDeletedEdgesList(gc->g);
	_RdbSaveDeletedEntities_v12(rdb, gc, deleted_edges_to_encode, deleted_edges_list);
}

void RdbSaveNodes_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t nodes_to_encode
) {
	// Format:
	// Node Format * nodes_to_encode:
	//  ID
	//  #labels M
	//  (labels) X M
	//  #properties N
	//  (name, value type, value) X N

	if(nodes_to_encode == 0) return;
	// get graph's node count
	uint64_t graph_nodes = Graph_NodeCount(gc->g);
	// get the number of nodes already encoded
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// get datablock iterator from context,
	// already set to offset by a previous encodeing of nodes, or create new one
	DataBlockIterator *iter = GraphEncodeContext_GetDatablockIterator(gc->encoding_context);
	if(!iter) {
		iter = Graph_ScanNodes(gc->g);
		GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	}

	for(uint64_t i = 0; i < nodes_to_encode; i++) {
		GraphEntity e;
		e.attributes = (AttributeSet *)DataBlockIterator_Next(iter, &e.id);
		_RdbSaveNode_v12(rdb, gc, &e);
	}

	// check if done encodeing nodes
	if(offset + nodes_to_encode == graph_nodes) {
		DataBlockIterator_Free(iter);
		iter = NULL;
		GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	}
}

void RdbSaveEdges_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t edges_to_encode
) {
	// Format:
	// Edge format * edges_to_encode:
	//  edge ID
	//  edge properties

	GrB_Info info;
	UNUSED(info);

	if(edges_to_encode == 0) return;

	// get graph's edge count
	uint64_t graph_edges = Graph_EdgeCount(gc->g);

	// get the number of edges already encoded
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// get datablock iterator from context,
	// already set to offset by a previous encodeing of nodes, or create new one
	DataBlockIterator *iter = GraphEncodeContext_GetDatablockIterator(gc->encoding_context);
	if(!iter) {
		iter = Graph_ScanEdges(gc->g);
		GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	}

	for(uint64_t i = 0; i < edges_to_encode; i++) {
		GraphEntity e;
		e.attributes = (AttributeSet *)DataBlockIterator_Next(iter, &e.id);
		_RdbSaveEdge_v12(rdb, &e);
	}

	// check if done encodeing nodes
	if(offset + edges_to_encode == graph_edges) {
		DataBlockIterator_Free(iter);
		iter = NULL;
		GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	}
}

void RdbSaveGraphTopology_v12
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t edges_to_encode
) {
	// Format:
	// Edge format * edges_to_encode:
	//  relation type
	//  source node ID
	//  destination node ID
	//  # edge count
	//  edge ID * N

	GrB_Info info;
	UNUSED(info);

	if(edges_to_encode == 0) return;

	// get graph's edge count
	uint64_t graph_edges = Graph_EdgeCount(gc->g);

	// get the number of edges already encoded
	uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

	// count the edges that will be encoded in this phase
	uint64_t encoded_edges = 0;

	// get current relation matrix
	uint r = GraphEncodeContext_GetCurrentRelationID(gc->encoding_context);

	RG_Matrix M = Graph_GetRelationMatrix(gc->g, r, false);

	// get matrix tuple iterator from context
	// already set to the next entry to fetch
	// for previous edge encide or create new one
	RG_MatrixTupleIter *iter = GraphEncodeContext_GetMatrixTupleIterator(gc->encoding_context);
	if(!RG_MatrixTupleIter_is_attached(iter, M)) {
		info = RG_MatrixTupleIter_attach(iter, M);
		ASSERT(info == GrB_SUCCESS);
	}

	NodeID src;
	NodeID dest;

	uint relation_count = Graph_RelationTypeCount(gc->g);
	// write the required number of edges
	while(encoded_edges < edges_to_encode) {
		EdgeID edgeID;

		// try to get next tuple
		info = RG_MatrixTupleIter_next_UINT64(iter, &src, &dest, &edgeID);

		// if iterator is depleted
		// get new tuple from different matrix or finish encode
		while(info == GxB_EXHAUSTED) {
			// proceed to next relation matrix
			r++;

			// if done iterating over all the matrices, jump to finish
			if(r == relation_count) goto finish;

			// get matrix and set iterator
			M = Graph_GetRelationMatrix(gc->g, r, false);
			info = RG_MatrixTupleIter_attach(iter, M);
			ASSERT(info == GrB_SUCCESS);
			info = RG_MatrixTupleIter_next_UINT64(iter, &src, &dest, &edgeID);
		}
		
		ASSERT(info == GrB_SUCCESS);

		RedisModule_SaveUnsigned(rdb, r);
		RedisModule_SaveUnsigned(rdb, src);
		RedisModule_SaveUnsigned(rdb, dest);
		if(SINGLE_EDGE(edgeID)) {
			RedisModule_SaveUnsigned(rdb, 1);
			RedisModule_SaveUnsigned(rdb, edgeID);
		} else {
			EdgeID * edges_array = (EdgeID *)(CLEAR_MSB(edgeID));
			uint edge_count = array_len(edges_array);
			RedisModule_SaveUnsigned(rdb, edge_count);
			for (uint i = 0; i < edge_count; i++) {
				RedisModule_SaveUnsigned(rdb, edges_array[i]);
			}
		}

		encoded_edges++;
	}

finish:
	// check if done encoding edges
	if(offset + edges_to_encode == graph_edges) {
		RG_MatrixTupleIter_detach(iter);
	}

	// update context
	GraphEncodeContext_SetCurrentRelationID(gc->encoding_context, r);
}
