/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "graphcontext.h"
#include "../util/rax_extensions.h"
#include "../configuration/config.h"
#include "../commands/execution_ctx.h"

// perform a deep copy of a graph context, providing it with a new name
GraphContext *GraphContext_Clone
(
	const char *graph_name,
	const GraphContext *gc
) {
	RedisModule_Log(NULL, REDISMODULE_LOGLEVEL_DEBUG, "cloning graph %s",
			gc->graph_name);

	GraphContext *clone = rm_calloc(1, sizeof(GraphContext));

	clone->version          = gc->version;
	clone->slowlog          = SlowLog_New();
	clone->ref_count        = 0;
	clone->attributes       = raxClone(gc->attributes);
	clone->encoding_context = GraphEncodeContext_New();
	clone->decoding_context = GraphDecodeContext_New();

	array_clone_with_cb(clone->string_mapping, gc->string_mapping, rm_strdup);

	clone->g = Graph_Clone(gc->g);
	clone->graph_name = rm_strdup(graph_name);

	//--------------------------------------------------------------------------
	// duplicate schemas and indices
	//--------------------------------------------------------------------------

	uint node_schema_count = array_len(gc->node_schemas);
	clone->node_schemas = array_new(Schema*, node_schema_count);
	for(uint i = 0; i < node_schema_count; i ++) {
		Schema *s = gc->node_schemas[i];
		array_append(clone->node_schemas, Schema_Clone(s));
	}

	uint relation_schema_count = array_len(gc->relation_schemas);
	clone->relation_schemas = array_new(Schema*, relation_schema_count);
	for(uint i = 0; i < relation_schema_count; i ++) {
		Schema *s = gc->relation_schemas[i];
		array_append(clone->relation_schemas, Schema_Clone(s));
	}

	// construct indices
	for(uint i = 0; i < node_schema_count; i ++) {
		Schema *s = clone->node_schemas[i];
		Index *idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx != NULL) {
			Index_Construct(idx, clone->g);
		}

		idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
		if(idx != NULL) {
			Index_Construct(idx, clone->g);
		}
	}

	for(uint i = 0; i < relation_schema_count; i ++) {
		Schema *s = clone->relation_schemas[i];
		Index *idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
		if(idx != NULL) {
			Index_Construct(idx, clone->g);
		}

		idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
		if(idx != NULL) {
			Index_Construct(idx, clone->g);
		}
	}

	// initialize the read-write lock to protect access to the attributes rax
	assert(pthread_rwlock_init(&clone->_attribute_rwlock, NULL) == 0);

	// build the execution plans cache
	uint64_t cache_size;
	Config_Option_get(Config_CACHE_SIZE, &cache_size);
	clone->cache = Cache_New(cache_size, (CacheEntryFreeFunc)ExecutionCtx_Free,
						  (CacheEntryCopyFunc)ExecutionCtx_Clone);

	Graph_SetMatrixPolicy(clone->g, SYNC_POLICY_FLUSH_RESIZE);

	// register cloned graph-context at the module level
	GraphContext_RegisterWithModule(clone);

	return clone;
}

