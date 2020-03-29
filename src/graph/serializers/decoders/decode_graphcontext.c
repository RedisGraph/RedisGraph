/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_graphcontext.h"

#include "decode_graph_entities.h"
#include "decode_schema.h"
#include "../../../util/arr.h"
#include "../../../query_ctx.h"
#include "../../../util/rmalloc.h"
#include "../../../slow_log/slow_log.h"

static GraphContext *_GetOrCreateGraphContext(RedisModuleIO *rdb) {
	// Graph name
	char *graph_name =  RedisModule_LoadStringBuffer(rdb, NULL);
	// Total keys representing the graph.
	uint64_t key_number = RedisModule_LoadUnsigned(rdb);
	GraphContext *gc = GraphContexted_GetRegistredGraphContext(graph_name);
	if(!gc) {
		gc = rm_calloc(1, sizeof(GraphContext));
		// Graph context defaults
		gc->index_count = 0;
		gc->attributes = raxNew();
		gc->string_mapping = array_new(char *, 64);
		gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
		gc->slowlog = SlowLog_New();
		gc->graph_name = strdup(graph_name);
		gc->encoding_context = GraphEncodeContext_New(key_number);
		// While loading the graph, minimize matrix realloc and synchronization calls.
		Graph_SetMatrixPolicy(gc->g, RESIZE_TO_CAPACITY);
	}
	// Free the name string, as it either not in used or copied.
	RedisModule_Free(graph_name);
	// Set the thread-local GraphContext, as it will be accessed if we're decoding indexes.
	QueryCtx_SetGraphCtx(gc);
	return gc;
}

GraphContext *RdbLoadGraphContext(RedisModuleIO *rdb) {

	GraphContext *gc = _GetOrCreateGraphContext(rdb);
	EncodePhase encoded_phase =  RedisModule_LoadUnsigned(rdb);
	switch(encoded_phase) {
	case NODES:
		RdbLoadNodes(rdb, gc);
		break;
	case DELETED_NODES:
		RdbLoadDeletedNodes(rdb, gc);
		break;
	case EDGES:
		RdbLoadEdges(rdb, gc);
		break;
	case DELETED_EDGES:
		RdbLoadDeletedEdges(rdb, gc);
		break;
	case GRAPH_SCHEMA:
		RdbLoadGraphSchema(rdb, gc);
		break;
	default:
		assert(false && "Unkown encoding");
		break;
	}
	GraphEncodeContext_IncreaseProcessedCount(gc->encoding_context);
	if(GraphEncodeContext_Finished(gc->encoding_context)) {
		// Revert to default synchronization behavior
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
		Graph_ApplyAllPending(gc->g);
		GraphEncodeContext_Reset(gc->encoding_context);
	}
	QueryCtx_Free(); // Release thread-local variables.
	return gc;
}

