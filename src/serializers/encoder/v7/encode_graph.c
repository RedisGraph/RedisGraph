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
	 * Current payload type
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

	// Payload type
	RedisModule_SaveUnsigned(rdb, GraphEncodeContext_GetEncodeState(gc->encoding_context));
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

void RdbSaveGraph_v7(RedisModuleIO *rdb, void *value) {
	/* Encoding format for graph context and graph meta key
	 * Header
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

	// If it is the start of the encodeing, select the phase to start from.
	if(GraphEncodeContext_GetEncodeState(gc->encoding_context) == INIT) _SelectFirstState(gc);

	// Save header
	_RdbSaveHeader(rdb, gc);

	switch(GraphEncodeContext_GetEncodeState(gc->encoding_context)) {
	case NODES:
		RdbSaveNodes_v7(rdb, gc);
		break;
	case DELETED_NODES:
		RdbSaveDeletedNodes_v7(rdb, gc);
		break;
	case EDGES:
		RdbSaveEdges_v7(rdb, gc);
		break;
	case DELETED_EDGES:
		RdbSaveDeletedEdges_v7(rdb, gc);
		break;
	case GRAPH_SCHEMA:
		RdbSaveGraphSchema_v7(rdb, gc);
		break;
	default:
		assert(false && "Unkown encoding phase");
		break;
	}

	// Increase processed key count. If finished encoding, resert context.
	GraphEncodeContext_IncreaseProcessedCount(gc->encoding_context);
	if(GraphEncodeContext_Finished(gc->encoding_context)) {
		GraphEncodeContext_Reset(gc->encoding_context);
	}

	// If a lock was acquired, release it.
	if(_shouldAcquireLocks()) Graph_ReleaseLock(gc->g);
}
