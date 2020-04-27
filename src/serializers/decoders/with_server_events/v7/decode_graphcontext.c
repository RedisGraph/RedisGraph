/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v7.h"

// Module event handler functions declarations.
void ModuleEventHandler_IncreaseDecodingGraphsCount(void);
void ModuleEventHandler_DecreaseDecodingGraphsCount(void);

static GraphContext *_GetOrCreateGraphContext(char *graph_name) {

	GraphContext *gc = GraphContext_GetRegisteredGraphContext(graph_name);
	if(!gc) {
		// New graph is being decoded. Inform the module and create new graph context.
		ModuleEventHandler_IncreaseDecodingGraphsCount();
		gc = GraphContext_New(graph_name, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
		// While loading the graph, minimize matrix realloc and synchronization calls.
		Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);
	}
	// Free the name string, as it either not in used or copied.
	RedisModule_Free(graph_name);
	// Set the thread-local GraphContext, as it will be accessed if we're decoding indexes.
	QueryCtx_SetGraphCtx(gc);
	return gc;
}

GraphContext *RdbLoadGraphContext_v7(RedisModuleIO *rdb) {

	// Graph name
	char *graph_name =  RedisModule_LoadStringBuffer(rdb, NULL);
	// Total keys representing the graph.
	uint64_t key_number = RedisModule_LoadUnsigned(rdb);

	GraphContext *gc = _GetOrCreateGraphContext(graph_name);
	GraphDecodeContext_SetKeyCount(gc->decoding_context, key_number);
	EncodePhase encoded_phase =  RedisModule_LoadUnsigned(rdb);
	/* The decode process contains the decode operation of many meta keys, representing independent parts of the graph.
	 * Each key contains data on one of the following:
	 * 1. Nodes - The nodes that are currently valid in the graph.
	 * 2. Deleted nodes - Nodes that were deleted and there ids can be re-used. Used for exact replication of data black state.
	 * 3. Edges - The edges that are currently valid in the graph.
	 * 4. Deleted edges - Edges that were deleted and there ids can be re-used. Used for exact replication of data black state.
	 * 5. Graph schema - Propertoes, indices.
	 * The following switch checks which part of the graph the current key holds, and decodes it accordingly. */
	switch(encoded_phase) {
	case NODES:
		RdbLoadNodes_v7(rdb, gc);
		break;
	case DELETED_NODES:
		RdbLoadDeletedNodes_v7(rdb, gc);
		break;
	case EDGES:
		RdbLoadEdges_v7(rdb, gc);
		break;
	case DELETED_EDGES:
		RdbLoadDeletedEdges_v7(rdb, gc);
		break;
	case GRAPH_SCHEMA:
		RdbLoadGraphSchema_v7(rdb, gc);
		break;
	default:
		assert(false && "Unknown encoding");
		break;
	}
	GraphDecodeContext_IncreaseProcessedCount(gc->decoding_context);
	if(GraphDecodeContext_Finished(gc->decoding_context)) {
		// Revert to default synchronization behavior
		Graph_ApplyAllPending(gc->g);
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
		GraphDecodeContext_Reset(gc->decoding_context);
		// Graph has finished decoding, inform the module.
		ModuleEventHandler_DecreaseDecodingGraphsCount();
	}
	QueryCtx_Free(); // Release thread-local variables.
	return gc;
}

