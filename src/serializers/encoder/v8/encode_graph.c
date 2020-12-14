/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v8.h"

extern bool process_is_child; // Global variable declared in module.c

// Determine whether we are in the context of a bgsave, in which case
// the process is independent and should not acquire locks.
static inline bool _shouldAcquireLocks(void) {
	return !process_is_child;
}

static void _RdbSaveHeader(RedisModuleIO *rdb, GraphEncodeContext *ctx) {
	/* Header format:
	 * Graph name
	 * Node count
	 * Edge count
	 * Label matrix count
	 * Relation matrix count - N
	 * Does relationship Ri holds mutiple edges under a single entry X N 
	 * Number of graph keys (graph context key + meta keys)
	 */

	ASSERT(ctx != NULL);

	GraphEncodeHeader *header = &(ctx->header);

	// Graph name.
	RedisModule_SaveStringBuffer(rdb, header->graph_name, strlen(header->graph_name) + 1);

	// Node count.
	RedisModule_SaveUnsigned(rdb, header->node_count);

	// Edge count.
	RedisModule_SaveUnsigned(rdb, header->edge_count);

	// Label matrix count.
	RedisModule_SaveUnsigned(rdb, header->label_matrix_count);

	// Relation matrix count.
	RedisModule_SaveUnsigned(rdb, header->relationship_matrix_count);

	// Does relationship Ri holds mutiple edges under a single entry X N.
	for(int i = 0; i < header->relationship_matrix_count; i++) {
		// true if R[i] contain a multi edge entry
		RedisModule_SaveUnsigned(rdb, header->multi_edge[i]);
	}

	// Number of keys.
	RedisModule_SaveUnsigned(rdb, header->key_count);
}

// Returns the a state information regarding the number of entities required to encode in this state.
static PayloadInfo _StatePayloadInfo(GraphContext *gc, EncodeState state,
									 uint64_t offset, uint64_t entities_to_encode) {
	uint64_t required_entities_count = 0;
	switch(state) {
	case ENCODE_STATE_NODES:
		required_entities_count = Graph_NodeCount(gc->g);
		break;
	case ENCODE_STATE_DELETED_NODES:
		required_entities_count = Graph_DeletedNodeCount(gc->g);
		break;
	case ENCODE_STATE_EDGES:
		required_entities_count = Graph_EdgeCount(gc->g);
		break;
	case ENCODE_STATE_DELETED_EDGES:
		required_entities_count = Graph_DeletedEdgeCount(gc->g);
		break;
	case ENCODE_STATE_GRAPH_SCHEMA:
		required_entities_count = 1;
		break;
	default:
		ASSERT(false && "Unknown encoding state in _CurrentStatePayloadInfo");
		break;
	}
	PayloadInfo payload_info;
	payload_info.state = state;
	/* When this state will be encoded, the number of entities to encode is the minimum between the number of entities to encode and
	 * the remaining entities left to encode from the same type. */
	payload_info.entities_count = MIN(entities_to_encode, required_entities_count - offset);
	return payload_info;
}

// This function saves the key content schema and returns it so the encoder can know how to encode the key.
static PayloadInfo *_RdbSaveKeySchema(RedisModuleIO *rdb, GraphContext *gc) {
	/*  Format:
	 *  #Number of payloads info - N
	 *  N * Payload info:
	 *      Encode state
	 *      Number of entities encoded in this state.
	 */
	PayloadInfo *payloads = array_new(PayloadInfo, 1);
	// Get current encoding state.
	EncodeState current_state = GraphEncodeContext_GetEncodeState(gc->encoding_context);
	// If it is the start of the encodeing, set the state to be NODES.
	if(current_state == ENCODE_STATE_INIT) current_state = ENCODE_STATE_NODES;

	uint64_t remaining_entities;
	Config_Option_get(Config_VKEY_MAX_ENTITY_COUNT, &remaining_entities);

	// No limit on the entities, the graph is encoded in one key.
	if(remaining_entities == VKEY_ENTITY_COUNT_UNLIMITED) {
		for(uint state = ENCODE_STATE_NODES; state < ENCODE_STATE_FINAL; state++) {
			payloads = array_append(payloads, _StatePayloadInfo(gc, state, 0, VKEY_ENTITY_COUNT_UNLIMITED));
		}
	} else {
		// Check if this is the last key
		bool last_key = GraphEncodeContext_GetProcessedKeyCount(gc->encoding_context) ==
						(GraphEncodeContext_GetKeyCount(gc->encoding_context) - 1);
		if(last_key) remaining_entities = VKEY_ENTITY_COUNT_UNLIMITED;

		// Get the current state encoded entities count.
		uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);

		// While there are still remaining entities to encode in this key and the state is valid.
		while(remaining_entities > 0 && current_state < ENCODE_STATE_FINAL) {
			// Get the current state payload info, with respect to offset.
			PayloadInfo current_state_payload_info = _StatePayloadInfo(gc, current_state, offset,
																	   remaining_entities);
			payloads = array_append(payloads, current_state_payload_info);
			if(!last_key) remaining_entities -= current_state_payload_info.entities_count;
			if(remaining_entities > 0) {
				offset = 0; // New state offset is 0.
				current_state++; // Advance in the states.
			}
		}
	}

	// Save the number of payloads.
	uint payloads_count = array_len(payloads);
	RedisModule_SaveUnsigned(rdb, payloads_count);
	for(uint i = 0; i < payloads_count; i++) {
		// For each payload, save its type and the number of entities it contains.
		PayloadInfo payload_info = payloads[i];
		RedisModule_SaveUnsigned(rdb, payload_info.state);
		RedisModule_SaveUnsigned(rdb, payload_info.entities_count);
	}

	return payloads;
}

void RdbSaveGraph_v8(RedisModuleIO *rdb, void *value) {
	/* Encoding format for graph context and graph meta key:
	 *  Header
	 *  Payload(s) count: N
	 *  Key content X N:
	 *      Payload type (Nodes / Edges / Deleted nodes/ Deleted edges/ Graph schema)
	 *      Entities in payload
	 *  Payload(s) X N
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
	 */

	GraphContext *gc = value;

	// Acquire a read lock if we're not in a thread-safe context.
	if(_shouldAcquireLocks()) Graph_AcquireReadLock(gc->g);

	EncodeState current_state = GraphEncodeContext_GetEncodeState(gc->encoding_context);

	if(current_state == ENCODE_STATE_INIT) {
		// Inital state, populate encoding context header
		GraphEncodeContext_InitHeader(gc->encoding_context, gc->graph_name, gc->g);
	}

	// Save header
	_RdbSaveHeader(rdb, gc->encoding_context);

	// Save payloads info for this key and retrive the key schema.
	PayloadInfo *key_schema = _RdbSaveKeySchema(rdb, gc);

	uint payloads_count = array_len(key_schema);
	for(uint i = 0; i < payloads_count; i++) {
		// If the current key encoding more than one payload type, payloads count >1 and we are in a new state, zero the entities count.
		if(i > 0) GraphEncodeContext_SetProcessedEntitiesOffset(gc->encoding_context, 0);
		PayloadInfo payload = key_schema[i];
		switch(payload.state) {
		case ENCODE_STATE_NODES:
			RdbSaveNodes_v8(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_DELETED_NODES:
			RdbSaveDeletedNodes_v8(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_EDGES:
			RdbSaveEdges_v8(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_DELETED_EDGES:
			RdbSaveDeletedEdges_v8(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_GRAPH_SCHEMA:
			RdbSaveGraphSchema_v8(rdb, gc);
			break;
		default:
			ASSERT(false && "Unknown encoding phase");
			break;
		}
		// Save the current state and the number of encoded entities.
		GraphEncodeContext_SetEncodeState(gc->encoding_context, payload.state);
		uint64_t offset = GraphEncodeContext_GetProcessedEntitiesOffset(gc->encoding_context);
		GraphEncodeContext_SetProcessedEntitiesOffset(gc->encoding_context,
													  payload.entities_count + offset);
	}
	array_free(key_schema);

	// Increase processed key count. If finished encoding, reset context.
	GraphEncodeContext_IncreaseProcessedKeyCount(gc->encoding_context);
	if(GraphEncodeContext_Finished(gc->encoding_context)) {
		GraphEncodeContext_Reset(gc->encoding_context);
		RedisModuleCtx *ctx = RedisModule_GetContextFromIO(rdb);
		RedisModule_Log(ctx, "notice", "Done encoding graph %s", gc->graph_name);
	}

	// If a lock was acquired, release it.
	if(_shouldAcquireLocks()) Graph_ReleaseLock(gc->g);
}

