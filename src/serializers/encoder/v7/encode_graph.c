/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v7.h"

extern bool process_is_child; // Global variable declared in module.c

// Determine whether we are in the context of a bgsave, in which case
// the process is independent and should not acquire locks.
static inline bool _shouldAcquireLocks(void) {
	return !process_is_child;
}

static void _RdbSaveHeader(RedisModuleIO *rdb, GraphContext *gc) {
	/* Header format:
	 * Graph name
	 * Node count
	 * Edge count
	 * Label matrix count
	 * Relation matrix count
	 * Number of graph keys (graph context key + meta keys)
	 */

	// Graph name.
	RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

	// Node count.
	RedisModule_SaveUnsigned(rdb, Graph_NodeCount(gc->g));

	// Edge count.
	RedisModule_SaveUnsigned(rdb, Graph_EdgeCount(gc->g));

	// Label matrix count
	RedisModule_SaveUnsigned(rdb, array_len(gc->g->labels));

	// Relation matrix count
	RedisModule_SaveUnsigned(rdb, array_len(gc->g->relations));

	// Number of keys
	RedisModule_SaveUnsigned(rdb, GraphEncodeContext_GetKeyCount(gc->encoding_context));

}

// Select the first state to encode.
static void _SelectFirstState(GraphContext *gc) {
	// If there are nodes
	if(Graph_NodeCount(gc->g) > 0) {
		GraphEncodeContext_SetEncodeState(gc->encoding_context, NODES);
	} else if(array_len(gc->g->nodes->deletedIdx) > 0) {
		// If all nodes are deleted
		GraphEncodeContext_SetEncodeState(gc->encoding_context, DELETED_NODES);
	}
	// No nodes and no deleted nodes => no edges or deleted edges. Only schema.
	else {
		GraphEncodeContext_SetEncodeState(gc->encoding_context, GRAPH_SCHEMA);
	}
}

// Returns the a state information regarding the number of entities required to encode in this state.
static PayloadInfo _StatePayloadInfo(GraphContext *gc, EncodeState state,
									 uint64_t offset, uint64_t entities_to_encode) {
	uint64_t required_entities_count;
	switch(state) {
	case NODES:
		required_entities_count = Graph_NodeCount(gc->g);
		break;
	case DELETED_NODES:
		required_entities_count = Graph_DeletedNodeCount(gc->g);
		break;
	case EDGES:
		required_entities_count = Graph_EdgeCount(gc->g);
		break;
	case DELETED_EDGES:
		required_entities_count = Graph_DeletedEdgeCount(gc->g);
		break;
	case GRAPH_SCHEMA:
		required_entities_count = 0;
		break;
	default:
		assert(false && "Unkown encoding state in _CurrentStatePayloadInfo");
	}
	PayloadInfo payload_info;
	payload_info.state = state;
	/* When this state will be encoded, the number of entities to encode is the minimum between the number of entities to encode and
	 * the remaining entities left to encode from the same type. */
	payload_info.entities_count = MIN(entities_to_encode, required_entities_count - offset);
	return payload_info;
}

// This function save the key content schema and returns it so the encoder can now how to encode the key.
static PayloadInfo *_RdbSaveKeySchema(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	*  #Number of payloads info - N
	*  N * Payload info:
	*      Encode state
	*      Number of entities encoded in this state.
	*/
	PayloadInfo *payloads = array_new(PayloadInfo, 1);
	// Get current encoding state.
	EncodeState current_state = GraphEncodeContext_GetEncodeState(gc->encoding_context);
	// If it is the start of the encodeing, set the state to be NODES.
	if(current_state == INIT) current_state = NODES;
	uint64_t remaining_entities = Config_GetModuleConfig().entities_threshold;
	// No limit on the entities, the graph is encoded in one key.
	if(remaining_entities == UNLIMITED) {
		PayloadInfo nodes_info = {.state = NODES, .entities_count = Graph_NodeCount(gc->g)};
		PayloadInfo deleted_nodes_info = {.state = DELETED_NODES, .entities_count = Graph_DeletedNodeCount(gc->g)};
		PayloadInfo edges_info = {.state = EDGES, .entities_count = Graph_EdgeCount(gc->g)};
		PayloadInfo deleted_edges_info = {.state = DELETED_EDGES, .entities_count = Graph_DeletedEdgeCount(gc->g)};
		PayloadInfo graph_schema_info = {.state = GRAPH_SCHEMA, .entities_count = 0};
		payloads = array_append(payloads, nodes_info);
		payloads = array_append(payloads, deleted_nodes_info);
		payloads = array_append(payloads, edges_info);
		payloads = array_append(payloads, deleted_edges_info);
		payloads = array_append(payloads, graph_schema_info);
	} else {
		// Get the current state encoded entities count.
		uint64_t offset = GraphEncodeContext_GetProcessedEntitiesCount(gc->encoding_context);
		// While there are still remaining entities to encode in this key and the state is valid.
		while(remaining_entities > 0 && current_state <= GRAPH_SCHEMA) {
			// Get the current state payload info, with respect to offset.
			PayloadInfo current_state_payload_info = _StatePayloadInfo(gc, current_state, offset,
																	   remaining_entities);
			payloads = array_append(payloads, current_state_payload_info);
			remaining_entities -= current_state_payload_info.entities_count;
			current_state++; // Advance in the states.
			offset = 0; // New state offset is 0.
		}
	}

	uint payloads_count = array_len(payloads);
	// Save the number of key payloads.
	RedisModule_SaveUnsigned(rdb, payloads_count);
	for(uint i = 0; i < payloads_count; i++) {
		// For each payload, save its type and the number of entities it contains.
		PayloadInfo payload_info = payloads[i];
		RedisModule_SaveUnsigned(rdb, payload_info.state);
		RedisModule_SaveUnsigned(rdb, payload_info.entities_count);
	}
	return payloads;
}

void RdbSaveGraph_v7(RedisModuleIO *rdb, void *value) {
	/* Encoding format for graph context and graph meta key
	 * Header
	 * Payload schema
	 * Payload - Nodes / Edges / Deleted nodes/ Deleted edges/ Graph schema
	 *
	 * This function will encode each payload type (if needed) in the following order:
	 * 1. Nodes
	 * 2. Deleted nodes
	 * 3. Edges
	 * 4. Deleted edges
	 * 5. Graph schema.
	 *
	 * Each payload type can spread over one or more keys. For example: A graph with 200,000 nodes, and the number of entities per payload
	 * is 100,000 then there will be two nodes payloads, each containing 100,000 nodes, encoded into two different RDB meta keys.
	 *
	 * Each encoding phase finished encoded chooses the next encoding phase according to the graph's data.
	 */

	GraphContext *gc = value;

	// Acquire a read lock if we're not in a thread-safe context.
	if(_shouldAcquireLocks()) Graph_AcquireReadLock(gc->g);

	// Save header
	_RdbSaveHeader(rdb, gc);

	// Save payloads info for this key and retrive the key schema.
	PayloadInfo *key_schema = _RdbSaveKeySchema(rdb, gc);

	uint payloads_count = array_len(key_schema);
	for(uint i = 0; i < payloads_count; i++) {
		// If the current key encoding more than one payload type, payloads count >1 and we are in a new state, zero the entities count.
		if(i > 0) GraphEncodeContext_SetProcessedEntitiesCount(gc->encoding_context, 0);
		PayloadInfo payload = key_schema[i];
		switch(payload.state) {
		case NODES:
			RdbSaveNodes_v7(rdb, gc, payload.entities_count);
			break;
		case DELETED_NODES:
			RdbSaveDeletedNodes_v7(rdb, gc, payload.entities_count);
			break;
		case EDGES:
			RdbSaveEdges_v7(rdb, gc, payload.entities_count);
			break;
		case DELETED_EDGES:
			RdbSaveDeletedEdges_v7(rdb, gc, payload.entities_count);
			break;
		case GRAPH_SCHEMA:
			RdbSaveGraphSchema_v7(rdb, gc);
			break;
		default:
			assert(false && "Unkown encoding phase");
			break;
		}
		// Save the current state and the number of encoded entities.
		GraphEncodeContext_SetEncodeState(gc->encoding_context, payload.state);
		uint64_t offset = GraphEncodeContext_GetProcessedEntitiesCount(gc->encoding_context);
		GraphEncodeContext_SetProcessedEntitiesCount(gc->encoding_context, payload.entities_count + offset);
	}
	array_free(key_schema);

	// Increase processed key count. If finished encoding, resert context.
	GraphEncodeContext_IncreaseProcessedCount(gc->encoding_context);
	if(GraphEncodeContext_Finished(gc->encoding_context)) {
		GraphEncodeContext_Reset(gc->encoding_context);
	}

	// If a lock was acquired, release it.
	if(_shouldAcquireLocks()) Graph_ReleaseLock(gc->g);
}
