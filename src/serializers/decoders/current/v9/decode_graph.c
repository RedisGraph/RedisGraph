/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v9.h"

static GraphContext *_GetOrCreateGraphContext(char *graph_name) {
	GraphContext *gc = GraphContext_GetRegisteredGraphContext(graph_name);
	if(!gc) {
		// New graph is being decoded. Inform the module and create new graph context.
		gc = GraphContext_New(graph_name);
		// While loading the graph, minimize matrix realloc and synchronization calls.
		Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);
	}
	// Free the name string, as it either not in used or copied.
	RedisModule_Free(graph_name);

	// Set the GraphCtx in thread-local storage.
	QueryCtx_SetGraphCtx(gc);

	return gc;
}

/* The first initialization of the graph data structure guarantees that there will be no further re-allocation
 * of data blocks and matrices since they are all in the appropriate size. */
static void _InitGraphDataStructure(Graph *g, uint64_t node_count, uint64_t edge_count,
									uint64_t label_count,  uint64_t relation_count) {
	Graph_AllocateNodes(g, node_count);
	Graph_AllocateEdges(g, edge_count);
	for(uint64_t i = 0; i < label_count; i++) Graph_AddLabel(g);
	for(uint64_t i = 0; i < relation_count; i++) Graph_AddRelationType(g);
}

static void _EnableMultiEdgeSupport(Graph *g) {
	uint n = Graph_RelationTypeCount(g);
	for(uint i = 0; i < n; i++) g->relations[i]->allow_multi_edge = true;
}

static GraphContext *_DecodeHeader(RedisModuleIO *rdb) {
	/* Header format:
	 * Graph name
	 * Node count
	 * Edge count
	 * Label matrix count
	 * Relation matrix count - N
	 * Does relationship matrix Ri holds mutiple edges under a single entry X N
	 * Number of graph keys (graph context key + meta keys)
	 */

	// Graph name
	char *graph_name = RedisModule_LoadStringBuffer(rdb, NULL);

	// Each key header contains the following: #nodes, #edges, #labels matrices, #relation matrices
	uint64_t node_count = RedisModule_LoadUnsigned(rdb);
	uint64_t edge_count = RedisModule_LoadUnsigned(rdb);
	uint64_t label_count = RedisModule_LoadUnsigned(rdb);
	uint64_t relation_count = RedisModule_LoadUnsigned(rdb);
	uint64_t multi_edge[relation_count];

	for(uint i = 0; i < relation_count; i++) {
		multi_edge[i] = RedisModule_LoadUnsigned(rdb);
	}

	// Total keys representing the graph.
	uint64_t key_number = RedisModule_LoadUnsigned(rdb);

	GraphContext *gc = _GetOrCreateGraphContext(graph_name);
	Graph *g = gc->g;
	// If it is the first key of this graph, allocate all the data structures,
	// with the appropriate dimensions
	if(GraphDecodeContext_GetProcessedKeyCount(gc->decoding_context) == 0) {
		_InitGraphDataStructure(gc->g, node_count, edge_count, label_count, relation_count);

		// Mark relationship matrices for support of multi-edge entries
		for(uint i = 0; i < relation_count; i++) {
			// Enable/Disable support for multi-edge
			// we will enable support for multi-edge on all relationship
			// matrices once we finish loading the graph
			g->relations[i]->allow_multi_edge = multi_edge[i];
		}

		GraphDecodeContext_SetKeyCount(gc->decoding_context, key_number);
	}

	return gc;
}

static PayloadInfo *_RdbLoadKeySchema(RedisModuleIO *rdb) {
	/* Format:
	*  #Number of payloads info - N
	*  N * Payload info:
	*      Encode state
	*      Number of entities encoded in this state.
	*/

	uint64_t payloads_count = RedisModule_LoadUnsigned(rdb);
	PayloadInfo *payloads = array_new(PayloadInfo, payloads_count);

	for(uint i = 0; i < payloads_count; i++) {
		// For each payload, load its type and the number of entities it contains.
		PayloadInfo payload_info;
		payload_info.state =  RedisModule_LoadUnsigned(rdb);
		payload_info.entities_count =  RedisModule_LoadUnsigned(rdb);
		payloads = array_append(payloads, payload_info);
	}
	return payloads;
}

GraphContext *RdbLoadGraph_v9(RedisModuleIO *rdb) {

	/* Key format:
	 *  Header
	 *  Payload(s) count: N
	 *  Key content X N:
	 *      Payload type (Nodes / Edges / Deleted nodes/ Deleted edges/ Graph schema)
	 *      Entities in payload
	 *  Payload(s) X N
	 * */

	GraphContext *gc = _DecodeHeader(rdb);
	// Load the key schema.
	PayloadInfo *key_schema = _RdbLoadKeySchema(rdb);

	/* The decode process contains the decode operation of many meta keys, representing independent parts of the graph.
	 * Each key contains data on one or more of the following:
	 * 1. Nodes - The nodes that are currently valid in the graph.
	 * 2. Deleted nodes - Nodes that were deleted and there ids can be re-used. Used for exact replication of data block state.
	 * 3. Edges - The edges that are currently valid in the graph.
	 * 4. Deleted edges - Edges that were deleted and there ids can be re-used. Used for exact replication of data block state.
	 * 5. Graph schema - Properties, indices.
	 * The following switch checks which part of the graph the current key holds, and decodes it accordingly. */
	uint payloads_count = array_len(key_schema);
	for(uint i = 0; i < payloads_count; i++) {
		PayloadInfo payload = key_schema[i];
		switch(payload.state) {
		case ENCODE_STATE_NODES:
			RdbLoadNodes_v9(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_DELETED_NODES:
			RdbLoadDeletedNodes_v9(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_EDGES:
			RdbLoadEdges_v9(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_DELETED_EDGES:
			RdbLoadDeletedEdges_v9(rdb, gc, payload.entities_count);
			break;
		case ENCODE_STATE_GRAPH_SCHEMA:
			RdbLoadGraphSchema_v9(rdb, gc);
			break;
		default:
			ASSERT(false && "Unknown encoding");
			break;
		}
	}
	array_free(key_schema);

	// Update decode context.
	GraphDecodeContext_IncreaseProcessedKeyCount(gc->decoding_context);
	// Before finalizing keep encountered meta keys names, for future deletion.
	const RedisModuleString *rm_key_name = RedisModule_GetKeyNameFromIO(rdb);
	const char *key_name = RedisModule_StringPtrLen(rm_key_name, NULL);
	// The virtual key name is not equal the graph name.
	if(strcmp(key_name, gc->graph_name) != 0) {
		GraphDecodeContext_AddMetaKey(gc->decoding_context, key_name);
	}

	if(GraphDecodeContext_Finished(gc->decoding_context)) {
		// Revert to default synchronization behavior
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
		Graph_ApplyAllPending(gc->g);
		// Set the thread-local GraphContext, as it will be accessed when creating indexes.
		QueryCtx_SetGraphCtx(gc);
		// Index the nodes when decoding ends.
		uint node_schemas_count = array_len(gc->node_schemas);
		for(uint i = 0; i < node_schemas_count; i++) {
			Schema *s = gc->node_schemas[i];
			if(s->index) Index_Construct(s->index);
			if(s->fulltextIdx) Index_Construct(s->fulltextIdx);
		}

		// Enable support for multi edge on all relationship matrices.
		_EnableMultiEdgeSupport(gc->g);

		QueryCtx_Free(); // Release thread-local variables.
		GraphDecodeContext_Reset(gc->decoding_context);

		RedisModuleCtx *ctx = RedisModule_GetContextFromIO(rdb);
		RedisModule_Log(ctx, "notice", "Done decoding graph %s", gc->graph_name);
	}
	return gc;
}

