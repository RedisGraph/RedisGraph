/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <sys/param.h>
#include <pthread.h>
#include "graphcontext.h"
#include "../RG.h"
#include "../util/arr.h"
#include "../util/uuid.h"
#include "../query_ctx.h"
#include "../redismodule.h"
#include "../util/rmalloc.h"
#include "../util/thpool/pools.h"
#include "../serializers/graphcontext_type.h"
#include "../commands/execution_ctx.h"

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;
extern uint aux_field_counter;
extern uint currently_decoding_graphs;
// GraphContext type as it is registered at Redis.
extern RedisModuleType *GraphContextRedisModuleType;

// Forward declarations.
static void _GraphContext_Free(void *arg);
static void _GraphContext_UpdateVersion(GraphContext *gc, const char *str);

static inline void _GraphContext_IncreaseRefCount(GraphContext *gc) {
	__atomic_fetch_add(&gc->ref_count, 1, __ATOMIC_RELAXED);
}

static inline void _GraphContext_DecreaseRefCount(GraphContext *gc) {
	// If the reference count is less than 0, the graph has been marked for deletion and no queries are active - free the graph.
	if(__atomic_sub_fetch(&gc->ref_count, 1, __ATOMIC_RELAXED) < 0) {
		bool async_delete;
		Config_Option_get(Config_ASYNC_DELETE, &async_delete);

		if(async_delete) {
			// Async delete
			ThreadPools_AddWorkWriter(_GraphContext_Free, gc);
		} else {
			// Sync delete
			_GraphContext_Free(gc);
		}
	}
}

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

// Creates and initializes a graph context struct.
GraphContext *GraphContext_New(const char *graph_name, size_t node_cap, size_t edge_cap) {
	GraphContext *gc = rm_malloc(sizeof(GraphContext));

	gc->version          = 0;  // initial graph version
	gc->slowlog          = SlowLog_New();
	gc->ref_count        = 0;  // no refences
	gc->attributes       = raxNew();
	gc->index_count      = 0;  // no indicies
	gc->string_mapping   = array_new(char *, 64);
	gc->encoding_context = GraphEncodeContext_New();
	gc->decoding_context = GraphDecodeContext_New();

	// initialize the graph's matrices and datablock storage
	gc->g = Graph_New(node_cap, edge_cap);
	gc->graph_name = rm_strdup(graph_name);

	// allocate the default space for schemas and indices
	gc->node_schemas = array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
	gc->relation_schemas = array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);

	// initialize the read-write lock to protect access to the attributes rax
	assert(pthread_rwlock_init(&gc->_attribute_rwlock, NULL) == 0);

	// build the execution plans cache
	uint64_t cache_size;
	Config_Option_get(Config_CACHE_SIZE, &cache_size);
	gc->cache = Cache_New(cache_size, (CacheEntryFreeFunc)ExecutionCtx_Free,
						  (CacheEntryCopyFunc)ExecutionCtx_Clone);

	Graph_SetMatrixPolicy(gc->g, SYNC_POLICY_FLUSH_RESIZE);
	QueryCtx_SetGraphCtx(gc);

	return gc;
}

/* _GraphContext_Create tries to get a graph context, and if it does not exists, create a new one.
 * The try-get-create flow is done when module global lock is acquired, to enforce consistency
 * while BGSave is called. */
static GraphContext *_GraphContext_Create(RedisModuleCtx *ctx, const char *graph_name,
										  size_t node_cap, size_t edge_cap) {
	// Create and initialize a graph context.
	GraphContext *gc = GraphContext_New(graph_name, node_cap, edge_cap);
	RedisModuleString *graphID = RedisModule_CreateString(ctx, graph_name, strlen(graph_name));

	RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_WRITE);
	// Set value in key.
	RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);
	// Register graph context for BGSave.
	GraphContext_RegisterWithModule(gc);

	RedisModule_FreeString(ctx, graphID);
	RedisModule_CloseKey(key);
	return gc;
}

/* In a sharded environment, there could be a race condition between the decoding of
 * the last key, and the last aux_fields, so both counters should be zeroed in order to verify
 * that the module replicated properly. */
static bool _GraphContext_IsModuleReplicating(void) {
	return aux_field_counter > 0 || currently_decoding_graphs > 0;
}

GraphContext *GraphContext_Retrieve(RedisModuleCtx *ctx, RedisModuleString *graphID, bool readOnly,
									bool shouldCreate) {
	if(_GraphContext_IsModuleReplicating()) {
		// The whole module is currently replicating, emit an error.
		RedisModule_ReplyWithError(ctx, "ERR RedisGraph module is currently replicating");
		return NULL;
	}
	GraphContext *gc = NULL;
	int rwFlag = readOnly ? REDISMODULE_READ : REDISMODULE_WRITE;

	RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, rwFlag);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		if(shouldCreate) {
			// Key doesn't exist, create it.
			const char *graphName = RedisModule_StringPtrLen(graphID, NULL);
			gc = _GraphContext_Create(ctx, graphName, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
		} else {
			// Key does not exist and won't be created, emit an error.
			RedisModule_ReplyWithError(ctx, "ERR Invalid graph operation on empty key");
		}
	} else if(RedisModule_ModuleTypeGetType(key) == GraphContextRedisModuleType) {
		gc = RedisModule_ModuleTypeGetValue(key);
	} else {
		// Key exists but is not a graph, emit an error.
		RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	RedisModule_CloseKey(key);

	if(gc) _GraphContext_IncreaseRefCount(gc);

	return gc;
}

void GraphContext_Release(GraphContext *gc) {
	ASSERT(gc);
	_GraphContext_DecreaseRefCount(gc);
}

void GraphContext_MarkWriter(RedisModuleCtx *ctx, GraphContext *gc) {
	RedisModuleString *graphID = RedisModule_CreateString(ctx, gc->graph_name, strlen(gc->graph_name));

	// Reopen only if key exists (do not re-create) make sure key still exists.
	RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_READ);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) goto cleanup;
	RedisModule_CloseKey(key);

	// Mark as writer.
	key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_WRITE);
	RedisModule_CloseKey(key);

cleanup:
	RedisModule_FreeString(ctx, graphID);
}

void GraphContext_Delete(GraphContext *gc) {
	/* We're here as a result of a call to:
	 * GRAPH.DELETE
	 * FLUSHALL
	 * DEL <graph_key>
	 * Redis decided to remove keys to save some space. */

	// Unregister graph object from global list.
	GraphContext_RemoveFromRegistry(gc);
	_GraphContext_DecreaseRefCount(gc);
}

const char *GraphContext_GetName(const GraphContext *gc) {
	ASSERT(gc != NULL);
	return gc->graph_name;
}

void GraphContext_Rename(GraphContext *gc, const char *name) {
	rm_free(gc->graph_name);
	gc->graph_name = rm_strdup(name);
}

XXH32_hash_t GraphContext_GetVersion(const GraphContext *gc) {
	ASSERT(gc != NULL);

	return gc->version;
}

// Update graph context version
static void _GraphContext_UpdateVersion(GraphContext *gc, const char *str) {
	ASSERT(gc != NULL);
	ASSERT(str != NULL);

	/* Update graph version by hashing 'str' representing the current
	 * addition to the graph schema: (Label, Relationship-type, Attribute)
	 *
	 * Using the current graph version as a seed, by doing so we avoid
	 * hashing the entire graph schema on each change, while guaranteeing the
	 * exact same version across a cluster: same graph version on both
	 * primary and replica shards. */

	XXH32_state_t *state = XXH32_createState();
	XXH32_reset(state, gc->version);
	XXH32_update(state, str, strlen(str));
	gc->version = XXH32_digest(state);
	XXH32_freeState(state);
}

//------------------------------------------------------------------------------
// Schema API
//------------------------------------------------------------------------------
// Find the ID associated with a label for schema and matrix access
int _GraphContext_GetLabelID(const GraphContext *gc, const char *label, SchemaType t) {
	// Choose the appropriate schema array given the entity type
	Schema **schemas = (t == SCHEMA_NODE) ? gc->node_schemas : gc->relation_schemas;

	// TODO optimize lookup
	for(uint32_t i = 0; i < array_len(schemas); i ++) {
		if(!strcmp(label, schemas[i]->name)) return i;
	}
	return GRAPH_NO_LABEL; // equivalent to GRAPH_NO_RELATION
}

unsigned short GraphContext_SchemaCount(const GraphContext *gc, SchemaType t) {
	ASSERT(gc);
	if(t == SCHEMA_NODE) return array_len(gc->node_schemas);
	else return array_len(gc->relation_schemas);
}

Schema *GraphContext_GetSchemaByID(const GraphContext *gc, int id, SchemaType t) {
	Schema **schemas = (t == SCHEMA_NODE) ? gc->node_schemas : gc->relation_schemas;
	if(id == GRAPH_NO_LABEL) return NULL;
	return schemas[id];
}

Schema *GraphContext_GetSchema(const GraphContext *gc, const char *label, SchemaType t) {
	int id = _GraphContext_GetLabelID(gc, label, t);
	return GraphContext_GetSchemaByID(gc, id, t);
}

Schema *GraphContext_AddSchema(GraphContext *gc, const char *label, SchemaType t) {
	int label_id;
	Schema *schema;

	if(t == SCHEMA_NODE) {
		label_id = Graph_AddLabel(gc->g);
		schema = Schema_New(SCHEMA_NODE, label_id, label);
		array_append(gc->node_schemas, schema);
	} else {
		label_id = Graph_AddRelationType(gc->g);
		schema = Schema_New(SCHEMA_EDGE, label_id, label);
		array_append(gc->relation_schemas, schema);
	}

	// new schema added, update graph version
	_GraphContext_UpdateVersion(gc, label);

	return schema;
}

const char *GraphContext_GetNodeLabel(const GraphContext *gc, Node *n) {
	int label_id = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(n));
	if(label_id == GRAPH_NO_LABEL) return NULL;
	return gc->node_schemas[label_id]->name;
}

const char *GraphContext_GetEdgeRelationType(const GraphContext *gc, Edge *e) {
	int reltype_id = Graph_GetEdgeRelation(gc->g, e);
	ASSERT(reltype_id != GRAPH_NO_RELATION);
	return gc->relation_schemas[reltype_id]->name;
}

uint GraphContext_AttributeCount(GraphContext *gc) {
	pthread_rwlock_rdlock(&gc->_attribute_rwlock);
	uint size = raxSize(gc->attributes);
	pthread_rwlock_unlock(&gc->_attribute_rwlock);
	return size;
}

Attribute_ID GraphContext_FindOrAddAttribute(GraphContext *gc, const char *attribute) {
	// Acquire a read lock for looking up the attribute.
	pthread_rwlock_rdlock(&gc->_attribute_rwlock);

	// See if attribute already exists.
	void *attribute_id = raxFind(gc->attributes, (unsigned char *)attribute, strlen(attribute));

	if(attribute_id == raxNotFound) {
		// We are writing to the shared GraphContext; release the held lock and re-acquire as a writer.
		pthread_rwlock_unlock(&gc->_attribute_rwlock);
		pthread_rwlock_wrlock(&gc->_attribute_rwlock);

		// Lookup the attribute again now that we are in a critical region.
		attribute_id = raxFind(gc->attributes, (unsigned char *)attribute, strlen(attribute));
		// If it has been set by another thread, use the retrieved value.
		if(attribute_id == raxNotFound) {
			// Otherwise, it will be assigned an ID equal to the current mapping size.
			attribute_id = (void *)raxSize(gc->attributes);
			// Insert the new attribute key and ID.
			raxInsert(gc->attributes,
					  (unsigned char *)attribute,
					  strlen(attribute),
					  attribute_id,
					  NULL);
			array_append(gc->string_mapping, rm_strdup(attribute));

			// new attribute been added, update graph version
			_GraphContext_UpdateVersion(gc, attribute);
		}
	}

	// Release the lock.
	pthread_rwlock_unlock(&gc->_attribute_rwlock);
	return (uintptr_t)attribute_id;
}

const char *GraphContext_GetAttributeString(GraphContext *gc, Attribute_ID id) {
	pthread_rwlock_rdlock(&gc->_attribute_rwlock);
	ASSERT(id < array_len(gc->string_mapping));
	const char *name = gc->string_mapping[id];
	pthread_rwlock_unlock(&gc->_attribute_rwlock);
	return name;
}

Attribute_ID GraphContext_GetAttributeID(GraphContext *gc, const char *attribute) {
	// Acquire a read lock for looking up the attribute.
	pthread_rwlock_rdlock(&gc->_attribute_rwlock);
	// Look up the attribute ID.
	void *id = raxFind(gc->attributes, (unsigned char *)attribute, strlen(attribute));
	// Release the lock.
	pthread_rwlock_unlock(&gc->_attribute_rwlock);

	if(id == raxNotFound) return ATTRIBUTE_NOTFOUND;

	return (uintptr_t)id;
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
bool GraphContext_HasIndices(GraphContext *gc) {
	ASSERT(gc != NULL);

	uint schema_count = array_len(gc->node_schemas);
	for(uint i = 0; i < schema_count; i++) {
		if(Schema_HasIndices(gc->node_schemas[i])) return true;
	}

	schema_count = array_len(gc->relation_schemas);
	for(uint i = 0; i < schema_count; i++) {
		if(Schema_HasIndices(gc->relation_schemas[i])) return true;
	}
	
	return false;
}
Index *GraphContext_GetIndexByID(const GraphContext *gc, int id,
					Attribute_ID *attribute_id, IndexType type, SchemaType t) {

	ASSERT(gc     !=  NULL);

	// Retrieve the schema for given id
	Schema *s = GraphContext_GetSchemaByID(gc, id, t);
	if(s == NULL) return NULL;

	return Schema_GetIndex(s, attribute_id, type);
}

Index *GraphContext_GetIndex(const GraphContext *gc, const char *label,
							 Attribute_ID *attribute_id, IndexType type,
							 SchemaType schema_type) {

	ASSERT(gc != NULL);
	ASSERT(label != NULL);

	// Retrieve the schema for this label
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);
	if(s == NULL) return NULL;

	return Schema_GetIndex(s, attribute_id, type);
}

int GraphContext_AddIndex
(
	Index **idx,
	GraphContext *gc,
	SchemaType schema_type,
	const char *label,
	const char *field,
	IndexType index_type
) {
	ASSERT(idx    !=  NULL);
	ASSERT(gc     !=  NULL);
	ASSERT(label  !=  NULL);
	ASSERT(field  !=  NULL);

	// Retrieve the schema for this label
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);
	if(s == NULL) s = GraphContext_AddSchema(gc, label, schema_type);

	int res = Schema_AddIndex(idx, s, field, index_type);
	ResultSet *result_set = QueryCtx_GetResultSet();
	ResultSet_IndexCreated(result_set, res);

	return res;
}

int GraphContext_DeleteIndex
(
	GraphContext *gc,
	SchemaType schema_type,
	const char *label,
	const char *field,
	IndexType type
) {
	ASSERT(gc     !=  NULL);
	ASSERT(label  !=  NULL);

	// retrieve the schema for this label
	int res = INDEX_FAIL;
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);

	if(s != NULL) {
		res = Schema_RemoveIndex(s, field, type);
		if(res != INDEX_FAIL) {
			// update resultset statistics
			ResultSet *result_set = QueryCtx_GetResultSet();
			ResultSet_IndexDeleted(result_set, res);
		}
	}

	return res;
}

// Delete all references to a node from any indices built upon its properties
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, Node *n) {
	Schema  *s  =  NULL;
	Graph   *g  =  gc->g;

	int label_id = NODE_GET_LABEL_ID(n, g);
	if(label_id == GRAPH_NO_LABEL) return;

	s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);

	// update any indices this entity is represented in
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx) Index_RemoveNode(idx, n);

	idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
	if(idx) Index_RemoveNode(idx, n);
}

void GraphContext_DeleteEdgeFromIndices(GraphContext *gc, Edge *e) {
	Schema  *s  =  NULL;
	Graph   *g  =  gc->g;

	int relation_id = EDGE_GET_RELATION_ID(e, g);

	s = GraphContext_GetSchemaByID(gc, relation_id, SCHEMA_EDGE);

	// update any indices this entity is represented in
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx) Index_RemoveEdge(idx, e);

	idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
	if(idx) Index_RemoveEdge(idx, e);
}

//------------------------------------------------------------------------------
// Functions for globally tracking GraphContexts
//------------------------------------------------------------------------------

// Register a new GraphContext for module-level tracking
void GraphContext_RegisterWithModule(GraphContext *gc) {
	// See if the graph context is not already in the keyspace.
	uint graph_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graph_count; i ++) {
		if(graphs_in_keyspace[i] == gc) return;
	}
	array_append(graphs_in_keyspace, gc);
}

GraphContext *GraphContext_GetRegisteredGraphContext(const char *graph_name) {
	GraphContext *gc = NULL;
	uint graph_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graph_count; i ++) {
		if(strcmp(graphs_in_keyspace[i]->graph_name, graph_name) == 0) {
			gc = graphs_in_keyspace[i];
			break;
		}
	}
	return gc;
}

// Delete a GraphContext reference from the global array
void GraphContext_RemoveFromRegistry(GraphContext *gc) {
	uint graph_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graph_count; i ++) {
		if(graphs_in_keyspace[i] == gc) {
			graphs_in_keyspace = array_del_fast(graphs_in_keyspace, i);
			break;
		}
	}
}

//------------------------------------------------------------------------------
// Slowlog API
//------------------------------------------------------------------------------

// Return slowlog associated with graph context.
SlowLog *GraphContext_GetSlowLog(const GraphContext *gc) {
	ASSERT(gc);
	return gc->slowlog;
}

//------------------------------------------------------------------------------
// Cache API
//------------------------------------------------------------------------------

// Return cache associated with graph context and current thread id.
Cache *GraphContext_GetCache(const GraphContext *gc) {
	ASSERT(gc != NULL);
	return gc->cache;
}

//------------------------------------------------------------------------------
// Free routine
//------------------------------------------------------------------------------

// Free all data associated with graph
static void _GraphContext_Free(void *arg) {
	GraphContext *gc = (GraphContext *)arg;
	uint len;

	// Disable matrix synchronization for graph deletion.
	Graph_SetMatrixPolicy(gc->g, SYNC_POLICY_NOP);
	Graph_Free(gc->g);

	//--------------------------------------------------------------------------
	// Free node schemas
	//--------------------------------------------------------------------------

	if(gc->node_schemas) {
		len = array_len(gc->node_schemas);
		for(uint32_t i = 0; i < len; i ++) {
			Schema_Free(gc->node_schemas[i]);
		}
		array_free(gc->node_schemas);
	}

	//--------------------------------------------------------------------------
	// Free relation schemas
	//--------------------------------------------------------------------------

	if(gc->relation_schemas) {
		len = array_len(gc->relation_schemas);
		for(uint32_t i = 0; i < len; i ++) {
			Schema_Free(gc->relation_schemas[i]);
		}
		array_free(gc->relation_schemas);
	}

	//--------------------------------------------------------------------------
	// Free attribute mappings
	//--------------------------------------------------------------------------

	if(gc->attributes) raxFree(gc->attributes);

	if(gc->string_mapping) {
		len = array_len(gc->string_mapping);
		for(uint32_t i = 0; i < len; i ++) {
			rm_free(gc->string_mapping[i]);
		}
		array_free(gc->string_mapping);
	}

	int res = pthread_rwlock_destroy(&gc->_attribute_rwlock);
	ASSERT(res == 0);

	if(gc->slowlog) SlowLog_Free(gc->slowlog);

	//--------------------------------------------------------------------------
	// Clear cache
	//--------------------------------------------------------------------------

	if(gc->cache) Cache_Free(gc->cache);

	GraphEncodeContext_Free(gc->encoding_context);
	GraphDecodeContext_Free(gc->decoding_context);
	rm_free(gc->graph_name);
	rm_free(gc);
}

