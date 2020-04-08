/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v7.h"
#include "../../../../../query_ctx.h"

static GraphContext *_GetOrCreateGraphContext(char *graph_name) {

	GraphContext *gc = GraphContexted_GetRegistredGraphContext(graph_name);
	if(!gc) {
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
	// Indicate the graph in decode.
	GraphContext_MarkInDecode(gc);
	EncodePhase encoded_phase =  RedisModule_LoadUnsigned(rdb);
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
		assert(false && "Unkown encoding");
		break;
	}
	GraphDecodeContext_IncreaseProcessedCount(gc->decoding_context);
	if(GraphDecodeContext_GetProccessedKeyCount(gc->decoding_context) == key_number) {
		// Revert to default synchronization behavior
		Graph_ApplyAllPending(gc->g);
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
		GraphDecodeContext_Reset(gc->decoding_context);
	}
	QueryCtx_Free(); // Release thread-local variables.
	return gc;
}

