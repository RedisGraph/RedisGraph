/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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
#include "../constraint/constraint.h"
#include "../serializers/graphcontext_type.h"
#include "../commands/execution_ctx.h"

// Global array tracking all extant GraphContexts (defined in module.c)
extern GraphContext **graphs_in_keyspace;
extern uint aux_field_counter;
// GraphContext type as it is registered at Redis.
extern RedisModuleType *GraphContextRedisModuleType;

// Forward declarations.
static void _GraphContext_Free(void *arg);
static void _GraphContext_UpdateVersion(GraphContext *gc, const char *str);

static uint64_t _count_indices_from_schemas(const Schema** schemas) {
	ASSERT(schemas);
	uint64_t count = 0;

	const uint32_t length = array_len(schemas);
	for (uint32_t i = 0; i < length; ++i) {
		const Schema *schema = schemas[i];
		ASSERT(schema);
		count += Schema_IndexCount(schema);
	}

	return count;
}

// delete a GraphContext reference from the `graphs_in_keyspace` global array
void _GraphContext_RemoveFromRegistry(GraphContext *gc) {
	uint graph_count = array_len(graphs_in_keyspace);
	for(uint i = 0; i < graph_count; i ++) {
		if(graphs_in_keyspace[i] == gc) {
			graphs_in_keyspace = array_del_fast(graphs_in_keyspace, i);
			break;
		}
	}
}

// increase graph context ref count by 1
inline void GraphContext_IncreaseRefCount
(
	GraphContext *gc
) {
	__atomic_fetch_add(&gc->ref_count, 1, __ATOMIC_RELAXED);
}

// decrease graph context ref count by 1
inline void GraphContext_DecreaseRefCount
(
	GraphContext *gc
) {
	// if the reference count is 0
	// the graph has been marked for deletion and no queries are active
	// free the graph
	if(__atomic_sub_fetch(&gc->ref_count, 1, __ATOMIC_RELAXED) == 0) {
		bool async_delete;
		Config_Option_get(Config_ASYNC_DELETE, &async_delete);

		// remove graph context from global `graphs_in_keyspace` array
		_GraphContext_RemoveFromRegistry(gc);

		if(async_delete) {
			// Async delete
			// add deletion task to pool using force mode
			// we can't lose this task in-case pool's queue is full
			ThreadPools_AddWorkWriter(_GraphContext_Free, gc, 1);
		} else {
			// Sync delete
			_GraphContext_Free(gc);
		}
	}
}

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

// creates and initializes a graph context struct
GraphContext *GraphContext_New
(
	const char *graph_name
) {
	GraphContext *gc = rm_malloc(sizeof(GraphContext));

	gc->version          = 0;  // initial graph version
	gc->slowlog          = SlowLog_New();
	gc->ref_count        = 0;  // no refences
	gc->attributes       = raxNew();
	gc->index_count      = 0;  // no indicies
	gc->string_mapping   = array_new(char *, 64);
	gc->encoding_context = GraphEncodeContext_New();
	gc->decoding_context = GraphDecodeContext_New();

	// read NODE_CREATION_BUFFER size from configuration
	// this value controls how much extra room we're willing to spend for:
	// 1. graph entity storage
	// 2. matrices dimensions
	size_t node_cap;
	size_t edge_cap;
	bool rc = Config_Option_get(Config_NODE_CREATION_BUFFER, &node_cap);
	assert(rc);
	edge_cap = node_cap;

	gc->g = Graph_New(node_cap, edge_cap);
	gc->graph_name = rm_strdup(graph_name);

	// allocate the default space for schemas and indices
	gc->node_schemas = array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
	gc->relation_schemas = array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);

	// initialize the read-write lock to protect access to the attributes rax
	int rc1 = pthread_rwlock_init(&gc->_attribute_rwlock, NULL);
	assert(rc1 == 0);

	// build the execution plans cache
	uint64_t cache_size;
	Config_Option_get(Config_CACHE_SIZE, &cache_size);
	gc->cache = Cache_New(cache_size, (CacheEntryFreeFunc)ExecutionCtx_Free,
						  (CacheEntryCopyFunc)ExecutionCtx_Clone);

	Graph_SetMatrixPolicy(gc->g, SYNC_POLICY_FLUSH_RESIZE);

	return gc;
}

/* _GraphContext_Create tries to get a graph context, and if it does not exists, create a new one.
 * The try-get-create flow is done when module global lock is acquired, to enforce consistency
 * while BGSave is called. */
static GraphContext *_GraphContext_Create
(
	RedisModuleCtx *ctx,
	const char *graph_name
) {
	// Create and initialize a graph context.
	GraphContext *gc = GraphContext_New(graph_name);
	RedisModuleString *graphID = RedisModule_CreateString(ctx, graph_name, strlen(graph_name));

	RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_WRITE);

	// set value in key
	RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);

	// register graph context for BGSave
	GraphContext_RegisterWithModule(gc);

	RedisModule_FreeString(ctx, graphID);
	RedisModule_CloseKey(key);
	return gc;
}

GraphContext *GraphContext_Retrieve
(
	RedisModuleCtx *ctx,
	RedisModuleString *graphID,
	bool readOnly,
	bool shouldCreate
) {
	// check if we're still replicating, if so don't allow access to the graph
	if(aux_field_counter > 0) {
		// the whole module is currently replicating, emit an error
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
			gc = _GraphContext_Create(ctx, graphName);
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

	if(gc) GraphContext_IncreaseRefCount(gc);

	return gc;
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

// get graph from graph context
Graph *GraphContext_GetGraph
(
	const GraphContext *gc
) {
	ASSERT(gc != NULL);
	
	return gc->g;
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

// enable all constraints
void GraphContext_EnableConstrains
(
	const GraphContext *gc
) {
	for(uint i = 0; i < array_len(gc->node_schemas); i ++) {
		Schema *s = gc->node_schemas[i];
		for(uint j = 0; j < array_len(s->constraints); j ++) {
			Constraint_Enable(s->constraints[j]);
		}
	}

	for(uint i = 0; i < array_len(gc->relation_schemas); i ++) {
		Schema *s = gc->relation_schemas[i];
		for(uint j = 0; j < array_len(s->constraints); j ++) {
			Constraint_Enable(s->constraints[j]);
		}
	}
}

// disable all constraints
void GraphContext_DisableConstrains
(
	GraphContext *gc
) {
	for(uint i = 0; i < array_len(gc->node_schemas); i ++) {
		Schema *s = gc->node_schemas[i];
		for(uint j = 0; j < array_len(s->constraints); j ++) {
			Constraint_Disable(s->constraints[j]);
		}
	}

	for(uint i = 0; i < array_len(gc->relation_schemas); i ++) {
		Schema *s = gc->relation_schemas[i];
		for(uint j = 0; j < array_len(s->constraints); j ++) {
			Constraint_Disable(s->constraints[j]);
		}
	}
}

Schema *GraphContext_GetSchemaByID(const GraphContext *gc, int id, SchemaType t) {
	Schema **schemas = (t == SCHEMA_NODE) ? gc->node_schemas : gc->relation_schemas;
	if(id == GRAPH_NO_LABEL) return NULL;
	return schemas[id];
}

Schema *GraphContext_GetSchema
(
	const GraphContext *gc,
	const char *label,
	SchemaType t
) {
	int id = _GraphContext_GetLabelID(gc, label, t);
	return GraphContext_GetSchemaByID(gc, id, t);
}

Schema *GraphContext_AddSchema
(
	GraphContext *gc,
	const char *label,
	SchemaType t
) {
	int id;
	Schema *schema;

	if(t == SCHEMA_NODE) {
		id = Graph_AddLabel(gc->g);
		schema = Schema_New(SCHEMA_NODE, id, label);
		array_append(gc->node_schemas, schema);
	} else {
		id = Graph_AddRelationType(gc->g);
		schema = Schema_New(SCHEMA_EDGE, id, label);
		array_append(gc->relation_schemas, schema);
	}

	// new schema added, update graph version
	_GraphContext_UpdateVersion(gc, label);

	return schema;
}

void GraphContext_RemoveSchema(GraphContext *gc, int schema_id, SchemaType t) {
	if(t == SCHEMA_NODE) {
		Schema *schema = gc->node_schemas[schema_id];
		Schema_Free(schema);
		gc->node_schemas = array_del(gc->node_schemas, schema_id);
	} else {
		Schema *schema = gc->relation_schemas[schema_id];
		Schema_Free(schema);
		gc->relation_schemas = array_del(gc->relation_schemas, schema_id);
	}
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

Attribute_ID GraphContext_FindOrAddAttribute
(
	GraphContext *gc,
	const char *attribute,
	bool *created
) {
	ASSERT(gc);

	bool created_flag = false;
	unsigned char *attr = (unsigned char*)attribute;
	uint l = strlen(attribute);

	// acquire a read lock for looking up the attribute
	pthread_rwlock_rdlock(&gc->_attribute_rwlock);

	// see if attribute already exists
	void *attribute_id = raxFind(gc->attributes, attr, l);

	if(attribute_id == raxNotFound) {
		// we are writing to the shared GraphContext
		// release the held lock and re-acquire as a writer
		pthread_rwlock_unlock(&gc->_attribute_rwlock);
		pthread_rwlock_wrlock(&gc->_attribute_rwlock);

		// lookup the attribute again now that we are in a critical region
		attribute_id = raxFind(gc->attributes, attr, l);

		// if set by another thread, use the retrieved value
		if(attribute_id == raxNotFound) {
			// otherwise, it will be assigned an ID
			// equal to the current mapping size
			attribute_id = (void *)raxSize(gc->attributes);
			// insert the new attribute key and ID
			raxInsert(gc->attributes, attr, l, attribute_id, NULL);
			array_append(gc->string_mapping, rm_strdup(attribute));
			created_flag = true;

			// new attribute been added, update graph version
			_GraphContext_UpdateVersion(gc, attribute);
		}
	}

	// release the lock
	pthread_rwlock_unlock(&gc->_attribute_rwlock);

	if(created) {
		*created = created_flag;
	}

	return (uintptr_t)attribute_id;
}

const char *GraphContext_GetAttributeString
(
	GraphContext *gc,
	Attribute_ID id
) {
	ASSERT(gc != NULL);
	ASSERT(id >= 0 && id < array_len(gc->string_mapping));

	pthread_rwlock_rdlock(&gc->_attribute_rwlock);
	const char *name = gc->string_mapping[id];
	pthread_rwlock_unlock(&gc->_attribute_rwlock);
	return name;
}

Attribute_ID GraphContext_GetAttributeID
(
	GraphContext *gc,
	const char *attribute
) {
	// Acquire a read lock for looking up the attribute.
	pthread_rwlock_rdlock(&gc->_attribute_rwlock);
	// Look up the attribute ID.
	void *id = raxFind(gc->attributes, (unsigned char *)attribute, strlen(attribute));
	// Release the lock.
	pthread_rwlock_unlock(&gc->_attribute_rwlock);

	if(id == raxNotFound) return ATTRIBUTE_ID_NONE;

	return (uintptr_t)id;
}

void GraphContext_RemoveAttribute
(
	GraphContext *gc,
	Attribute_ID id
) {
	ASSERT(gc);
	ASSERT(id == array_len(gc->string_mapping) - 1);
	pthread_rwlock_wrlock(&gc->_attribute_rwlock);
	const char *attribute = gc->string_mapping[id];
	int ret = raxRemove(gc->attributes,  (unsigned char *)attribute, strlen(attribute), NULL);
	ASSERT(ret == 1);
	rm_free(gc->string_mapping[id]);
	gc->string_mapping = array_del(gc->string_mapping, id);
	pthread_rwlock_unlock(&gc->_attribute_rwlock);
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
bool GraphContext_HasIndices(GraphContext *gc) {
	ASSERT(gc != NULL);

	return GraphContext_NodeIndexCount(gc) || GraphContext_EdgeIndexCount(gc);
}

uint64_t GraphContext_NodeIndexCount
(
	const GraphContext *gc
) {
	ASSERT(gc);
	return _count_indices_from_schemas((const Schema**)gc->node_schemas);
}

uint64_t GraphContext_EdgeIndexCount
(
	const GraphContext *gc
) {
	ASSERT(gc);
	return _count_indices_from_schemas((const Schema**)gc->relation_schemas);
}

// attempt to retrieve an index on the given label and attribute IDs
Index GraphContext_GetIndexByID
(
	const GraphContext *gc,        // graph context
	int lbl_id,                    // label / rel-type ID
	const Attribute_ID *attrs,     // attributes
	uint n,                        // attributes count
	IndexType idx_type,            // index type
	GraphEntityType entity_type    // schema type NODE / EDGE
) {
	// validations
	ASSERT(gc != NULL);
	ASSERT((attrs == NULL && n == 0) || (attrs != NULL && n > 0));

	// retrieve the schema for given id
	SchemaType st = (entity_type == GETYPE_NODE) ? SCHEMA_NODE : SCHEMA_EDGE;
	Schema *s = GraphContext_GetSchemaByID(gc, lbl_id, st);
	if(s == NULL) {
		return NULL;
	}

	return Schema_GetIndex(s, attrs, n, idx_type, false);
}

// attempt to retrieve an index on the given label and attribute
Index GraphContext_GetIndex
(
	const GraphContext *gc,
	const char *label,
	Attribute_ID *attrs,
	uint n,
	IndexType type,
	SchemaType schema_type
) {
	ASSERT(gc    != NULL);
	ASSERT(label != NULL);

	// Retrieve the schema for this label
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);
	if(s == NULL) return NULL;

	return Schema_GetIndex(s, attrs, n, type, false);
}

// create an exact match index for the given label and attribute
bool GraphContext_AddExactMatchIndex
(
	Index *idx,                 // [input/output] index created
	GraphContext *gc,           // graph context
	SchemaType schema_type,     // type of entities to index nodes/edges
	const char *label,          // label of indexed entities
	const char **fields_str,    // fields to index
	uint fields_count,          // number of fields to index
	bool should_reply           // should reply to client
) {
	ASSERT(idx    !=  NULL);
	ASSERT(gc     !=  NULL);
	ASSERT(label  !=  NULL);
	ASSERT(fields_str !=  NULL);
	ASSERT(fields_count > 0);

	// retrieve the schema for this label
	bool      index_changed = false;
	Schema    *s            = GraphContext_GetSchema(gc, label, schema_type);
	ResultSet *result_set   = should_reply ? QueryCtx_GetResultSet() : NULL;

	if(s == NULL) {
		// schema doesn't exists, create it
		s = GraphContext_AddSchema(gc, label, schema_type);
	}

	for(uint i = 0; i < fields_count; i++) {
		// create index field
		const char *field = fields_str[i];
		IndexField idx_field;
		Attribute_ID f_id = GraphContext_FindOrAddAttribute(gc, field, NULL);
		IndexField_Default(&idx_field, f_id, field);
		if(Schema_AddIndex(idx, s, &idx_field, IDX_EXACT_MATCH) == INDEX_OK) {
			index_changed = true;
			if(should_reply) {
				// update result-set
				ResultSet_IndexCreated(result_set, INDEX_OK);
			}
		}
	}

	// disable index if it was created
	// we don't call Index_Disable within Schema_AddIndex as multiple
	// field additions are still considered a "single" modification
	// of the index
	if(index_changed) {
		Index_Disable(*idx);
	} else {
		if(should_reply) {
			// update result-set
			ResultSet_IndexCreated(result_set, INDEX_FAIL);
		}
	}

	return index_changed;
}

// create a full text index for the given label and attribute
bool GraphContext_AddFullTextIndex
(
	Index *idx,             // [input/output] index created
	GraphContext *gc,        // graph context
	const char *label,       // label of indexed entities
	const char **fields,     // fields to index
	uint fields_count,       // number of fields to index
	double *weights,         // fields weights
	bool *nostems,           //
	const char **phonetics,  //
	char **stopwords,
	const char *language
) {
	ASSERT(idx    != NULL);
	ASSERT(gc     != NULL);
	ASSERT(label  != NULL);
	ASSERT(fields != NULL);
	ASSERT(fields_count > 0);

	// retrieve the schema for this label
	ResultSet *result_set   = QueryCtx_GetResultSet();
	bool      index_changed = false;
	Schema    *s            = GraphContext_GetSchema(gc, label, SCHEMA_NODE);

	if(s == NULL) {
		s = GraphContext_AddSchema(gc, label, SCHEMA_NODE);
	}

	for(uint i = 0; i < fields_count; i++) {
		const char* field     = fields[i];
		double      weight    = weights[i];
		bool        nostem    = nostems[i];
		const char  *phonetic = phonetics[i];

		IndexField index_field;
		Attribute_ID f_id = GraphContext_FindOrAddAttribute(gc, field, NULL);
		IndexField_New(&index_field, f_id, field, weight, nostem, phonetic);
		if(Schema_AddIndex(idx, s, &index_field, IDX_FULLTEXT) == INDEX_OK) {
			index_changed = true;
			// update result-set
			ResultSet_IndexCreated(result_set, INDEX_OK);
		}
	}

	if(stopwords != NULL) {
		Index_SetStopwords(*idx, stopwords);
	}

	if(language != NULL) {
		Index_SetLanguage(*idx, language);
	}

	// diable index if it was created
	if(index_changed) {
		Index_Disable(*idx);
	}

	return index_changed;
}

int GraphContext_DeleteIndex
(
	GraphContext *gc,
	SchemaType schema_type,
	const char *label,
	const char *field,
	IndexType type
) {
	ASSERT(gc    != NULL);
	ASSERT(label != NULL);

	// retrieve the schema for this label
	int res = INDEX_FAIL;
	Schema *s = GraphContext_GetSchema(gc, label, schema_type);

	if(s != NULL) {
		res = Schema_RemoveIndex(s, field, type);
		if(res == INDEX_OK) {
			// update resultset statistics
			ResultSet *result_set = QueryCtx_GetResultSet();
			ResultSet_IndexDeleted(result_set, res);
		}
	}

	return res;
}

//------------------------------------------------------------------------------
// Functions for globally tracking GraphContexts
//------------------------------------------------------------------------------

// register a new GraphContext for module-level tracking
void GraphContext_RegisterWithModule(GraphContext *gc) {

	// increase graph context ref count
	GraphContext_IncreaseRefCount(gc);

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
	if(gc->decoding_context == NULL || GraphDecodeContext_Finished(gc->decoding_context)) Graph_Free(gc->g);
	else Graph_PartialFree(gc->g);

	bool async_delete;
	Config_Option_get(Config_ASYNC_DELETE, &async_delete);

	RedisModuleCtx *ctx = NULL;
	if(async_delete) {
		ctx = RedisModule_GetThreadSafeContext(NULL);
		// GIL need to be acquire because RediSearch change Redis global data structure
		RedisModule_ThreadSafeContextLock(ctx);
	}

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

	if(async_delete) {
		RedisModule_ThreadSafeContextUnlock(ctx);
		RedisModule_FreeThreadSafeContext(ctx);
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

void GraphContext_LockForCommit
(
	RedisModuleCtx *ctx,
	GraphContext *gc
) {
    // aquire GIL
    RedisModule_ThreadSafeContextLock(ctx);

	// acquire graph write lock
	Graph_AcquireWriteLock(gc->g);
}

void GraphContext_UnlockCommit
(
	RedisModuleCtx *ctx,
	GraphContext *gc
) {
	// release graph R/W lock
	Graph_ReleaseLock(gc->g);

	// unlock GIL
	RedisModule_ThreadSafeContextUnlock(ctx);
}
