/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "encode_graph_entities.h"
#include "../../../util/arr.h"
#include "../../../datatypes/array.h"

extern uint64_t entities_threshold;

// Forward declerations.
static void _RdbSaveSIArray(RedisModuleIO *rdb, const SIValue array);

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

static void _RdbSaveSIArray(RedisModuleIO *rdb, const SIValue list) {
	/* saves array as
	   unsinged : array legnth
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

static void _RdbSaveEntity(RedisModuleIO *rdb, const Entity *e,  char **attr_map) {
	/* Format:
	 * #attributes N
	 * (name, value type, value) X N  */

	RedisModule_SaveUnsigned(rdb, e->prop_count);

	for(int i = 0; i < e->prop_count; i++) {
		EntityProperty attr = e->properties[i];
		const char *attr_name = attr_map[attr.id];
		RedisModule_SaveStringBuffer(rdb, attr_name, strlen(attr_name) + 1);
		_RdbSaveSIValue(rdb, &attr.value);
	}
}

static void _UpdatedEncodePhase(GraphContext *gc) {
	// Check if NODES encodeding phase is done
	if(GraphEncodeContext_GetProccessedNodes(gc->encoding_context) == Graph_NodeCount(gc->g)) {
		// Check if there is a need to encoded deleted nodes, or skip to edges.
		if(array_len(gc->g->nodes->deletedIdx) >
		   GraphEncodeContext_GetProccessedDeletedNodes(gc->encoding_context)) {
			GraphEncodeContext_SetEncodePhase(gc->encoding_context, DELETED_NODES);
		} else {
			// No nodes or deleted nodes left. Set state to edges.
			GraphEncodeContext_SetEncodePhase(gc->encoding_context, EDGES);
			// Check if need to encode edges, deleted edges or skip tp schema.
			if(GraphEncodeContext_GetProccessedEdges(gc->encoding_context) == Graph_EdgeCount(gc->g)) {
				// Check if there is a need to encoded deleted edges, or skip to schema.
				if(array_len(gc->g->edges->deletedIdx) >
				   GraphEncodeContext_GetProccessedDeletedEdges(gc->encoding_context)) {
					GraphEncodeContext_SetEncodePhase(gc->encoding_context, DELETED_EDGES);
				} else {
					GraphEncodeContext_SetEncodePhase(gc->encoding_context, GRAPH_SCHEMA);
				}
			}
		}
	}
}

void RdbSaveDeletedNodes(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #deleted nodes N
	 * node id X N */

	// Get graph's deleted node count.
	uint64_t deleted_nodes = array_len(gc->g->nodes->deletedIdx);
	// Get the number of deleted nodes already encoded.
	uint64_t encoded_deleted_nodes = GraphEncodeContext_GetProccessedDeletedNodes(gc->encoding_context);
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
	_UpdatedEncodePhase(gc);
}

void RdbSaveNodes(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #nodes
	 *      #labels M
	 *      (labels) X M
	 *      #properties N
	 *      (name, value type, value) X N */

	// Get graph's node count.
	uint64_t graph_nodes = Graph_NodeCount(gc->g);
	// Get the number of nodes already encoded.
	uint64_t encoded_nodes = GraphEncodeContext_GetProccessedNodes(gc->encoding_context);
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
		int l = Graph_GetNodeLabel(gc->g, e->id);

		// #labels, currently only one label per node.
		int label_count = (l == GRAPH_NO_LABEL) ? 0 : 1;
		RedisModule_SaveUnsigned(rdb, label_count);

		// (label)
		if(label_count) RedisModule_SaveUnsigned(rdb, l);

		// properties N
		// (name, value type, value) X N
		_RdbSaveEntity(rdb, e, gc->string_mapping);

	}

	// Check if done encodeing nodes.
	if(encoded_nodes + nodes_to_encode == graph_nodes) {
		DataBlockIterator_Free(iter);
		iter = NULL;
	}

	// Update context.
	GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	GraphEncodeContext_SetProcessedNodes(gc->encoding_context, encoded_nodes + nodes_to_encode);
	_UpdatedEncodePhase(gc);
}

void RdbSaveDeletedEdges(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #deleted edges N
	 * edge id X N */

	// Get graph's deleted edge count.
	uint64_t deleted_edges = array_len(gc->g->edges->deletedIdx);
	// Get the number of deleted edges already encoded.
	uint64_t encoded_deleted_edges = GraphEncodeContext_GetProccessedDeletedEdges(gc->encoding_context);
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
	_UpdatedEncodePhase(gc);
}

void RdbSaveEdges(RedisModuleIO *rdb, GraphContext *gc) {
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
	uint64_t encoded_edges = GraphEncodeContext_GetProccessedEdges(gc->encoding_context);
	// Calculate the number of edges required to encode in this phase.
	uint64_t edges_to_encode = graph_edges - encoded_edges;
	// If the required number is bigger than the allowed entities threshold, set it to be the threshold.
	if(edges_to_encode > entities_threshold) edges_to_encode = entities_threshold;
	// Get datablock iterator from context, or create new one.
	DataBlockIterator *iter = GraphEncodeContext_GetDatablockIterator(gc->encoding_context);
	if(!iter) iter = Graph_ScanEdges(gc->g);
	// #Nodes
	RedisModule_SaveUnsigned(rdb, edges_to_encode);
	for(uint64_t i = 0; i < edges_to_encode; i++) {
		Edge *e = (Edge *)DataBlockIterator_Next(iter);
		NodeID src = Edge_GetSrcNodeID(e);
		NodeID dest = Edge_GetDestNodeID(e);

		// Source node ID.
		RedisModule_SaveUnsigned(rdb, src);

		// Destination node ID.
		RedisModule_SaveUnsigned(rdb, dest);

		// Relation type.
		RedisModule_SaveUnsigned(rdb, e->relationID);

		// Edge properties.
		_RdbSaveEntity(rdb, e->entity, gc->string_mapping);
	}

	// Check if done encodeing edges.
	if(encoded_edges + edges_to_encode == graph_edges) {
		DataBlockIterator_Free(iter);
		iter = NULL;
	}

	// Update context.
	GraphEncodeContext_SetDatablockIterator(gc->encoding_context, iter);
	GraphEncodeContext_SetProcessedEdges(gc->encoding_context, encoded_edges + edges_to_encode);
	_UpdatedEncodePhase(gc);
}