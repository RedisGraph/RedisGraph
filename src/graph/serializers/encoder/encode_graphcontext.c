/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_graphcontext.h"
#include "encode_graph_entities.h"
#include "encode_schema.h"

extern bool process_is_child; // Global variable declared in module.c

// Determine whether we are in the context of a bgsave, in which case
// the process is independent and should not acquire locks.
static inline bool _shouldAcquireLocks(void) {
	return !process_is_child;
}



static void _RdbSaveHeader(RedisModuleIO *rdb, GraphContext *gc) {
	/* Header format:
	* Graph name
	* Number of graph keys (graph context key + meta keys)
	* Number of processed keys (current payload index)
	* Current payload type
	*/

	// Graph name.
	RedisModule_SaveStringBuffer(rdb, gc->graph_name, strlen(gc->graph_name) + 1);

	// Number of keys
	RedisModule_SaveUnsigned(rdb, GraphEncodeContext_GetKeyCount(gc->encoding_context));

	// Numbder of processed keys - current payload index
	RedisModule_SaveUnsigned(rdb, GraphEncodeContext_GetProccessedKeyCount(gc->encoding_context));

	// Payload type
	RedisModule_SaveUnsigned(rdb, GraphEncodeContext_GetEncodePhase(gc->encoding_context));
}


static void _SelectAndEncodeNextPhase(RedisModuleIO *rdb, GraphContext *gc) {
	// If there are nodes
	if(Graph_NodeCount(gc->g) > 0) {
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, NODES);
		RdbSaveNodes(rdb, gc);
	}
	// If all nodes are deleted
	else if(array_len(gc->g->nodes->deletedIdx) > 0) {
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, DELETED_NODES);
		RdbSaveDeletedNodes(rdb, gc);
	}
	// No nodes and no deleted nodes => no edges or deleted edges. Only schema.
	else {
		GraphEncodeContext_SetEncodePhase(gc->encoding_context, GRAPH_SCHEMA);
		RdbSaveGraphSchema(rdb, gc);
	}
}

void RdbSaveGraphContext(RedisModuleIO *rdb, void *value) {
	/* Encoding format for graph context and graph meta key
	 * Header
	 * Payload - Nodes / Edges / Deleted nodes/ Deleted edges/ Graph schema
	 *
	 * Payloads encoding order - Nodes => Deleted nodes (if needed) => Edges => Deleted edges (if needed) => Schema.
	 * Each encoding phase finished
	 */

	GraphContext *gc = value;

	// Acquire a read lock if we're not in a thread-safe context.
	if(_shouldAcquireLocks()) Graph_AcquireReadLock(gc->g);

	// Save header
	_RdbSaveHeader(rdb, gc);

	switch(GraphEncodeContext_GetEncodePhase(gc->encoding_context)) {
	case RESET:
		_SelectAndEncodeNextPhase(rdb, gc);
		break;
	case NODES:
		RdbSaveNodes(rdb, gc);
		break;
	case DELETED_NODES:
		RdbSaveDeletedNodes(rdb, gc);
		break;
	case EDGES:
		RdbSaveEdges(rdb, gc);
		break;
	case DELETED_EDGES:
		RdbSaveDeletedEdges(rdb, gc);
		break;
	case GRAPH_SCHEMA:
		RdbSaveGraphSchema(rdb, gc);
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
