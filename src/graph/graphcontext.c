/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <sys/param.h>
#include <pthread.h>
#include "graphcontext.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../redismodule.h"
#include "../util/rmalloc.h"
#include "../util/thpool/thpool.h"
#include "serializers/graphcontext_type.h"

// Global array tracking all extant GraphContexts (defined in module.c)
extern threadpool _thpool;
extern GraphContext **graphs_in_keyspace;

// Forward declarations.
static void _GraphContext_Free(void *arg);

static inline void _GraphContext_IncreaseRefCount(GraphContext *gc) {
	__atomic_fetch_add(&gc->ref_count, 1, __ATOMIC_RELAXED);
}

static inline void _GraphContext_DecreaseRefCount(GraphContext *gc) {
	// If the reference count is less than 0, the graph has been marked for deletion and no queries are active - free the graph.
	if(__atomic_sub_fetch(&gc->ref_count, 1, __ATOMIC_RELAXED) < 0)
		thpool_add_work(_thpool, _GraphContext_Free, gc);
}

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

// Creates and initializes a graph context struct.
static GraphContext *_GraphContext_New(const char *graph_name, size_t node_cap, size_t edge_cap) {
	GraphContext *gc = rm_malloc(sizeof(GraphContext));

	gc->ref_count = 0;      // No refences.
	gc->index_count = 0;    // No indicies.

	// Initialize the graph's matrices and datablock storage
	gc->g = Graph_New(node_cap, edge_cap);
	gc->graph_name = rm_strdup(graph_name);
	// Allocate the default space for schemas and indices
	gc->node_schemas = array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
	gc->relation_schemas = array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);

	gc->string_mapping = array_new(char *, 64);
	gc->attributes = raxNew();
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
	QueryCtx_SetGraphCtx(gc);

	return gc;
}

/* _GraphContext_Create tries to get a graph context, and if it does not exists, create a new one.
 * The try-get-create flow is done when module global lock is acquired, to enforce consistency
 * while BGSave is called. */
static GraphContext *_GraphContext_Create(RedisModuleCtx *ctx, const char *graph_name,
										  size_t node_cap, size_t edge_cap) {
	// Create and initialize a graph context.
	GraphContext *gc = _GraphContext_New(graph_name, node_cap, edge_cap);
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

GraphContext *GraphContext_Retrieve(RedisModuleCtx *ctx, RedisModuleString *graphID, bool readOnly,
									bool shouldCreate) {
	GraphContext *gc = NULL;
	int rwFlag = readOnly ? REDISMODULE_READ : REDISMODULE_WRITE;

	RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, rwFlag);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		// Key doesn't exists, create it.
		if(shouldCreate) {
			const char *graphName = RedisModule_StringPtrLen(graphID, NULL);
			gc = _GraphContext_Create(ctx, graphName, GRAPH_DEFAULT_NODE_CAP, GRAPH_DEFAULT_EDGE_CAP);
		}
	} else if(RedisModule_ModuleTypeGetType(key) == GraphContextRedisModuleType) {
		gc = RedisModule_ModuleTypeGetValue(key);
	}
	RedisModule_CloseKey(key);

	if(gc) {
		_GraphContext_IncreaseRefCount(gc);
		Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);
	}
	return gc;
}

void GraphContext_Release(GraphContext *gc) {
	assert(gc);
	_GraphContext_DecreaseRefCount(gc);
}

void GraphContext_MarkWriter(RedisModuleCtx *ctx, GraphContext *gc) {
	RedisModuleString *graphID = RedisModule_CreateString(ctx, gc->graph_name, strlen(gc->graph_name));

	// Reopen only if key exists (do not re-create) make sure key still exists.
	RedisModuleKey *key = RedisModule_OpenKey(ctx, graphID, REDISMODULE_READ);
	if(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) goto cleanup;

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
	assert(gc);
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
		schema = Schema_New(label, label_id);
		gc->node_schemas = array_append(gc->node_schemas, schema);
	} else {
		label_id = Graph_AddRelationType(gc->g);
		schema = Schema_New(label, label_id);
		gc->relation_schemas = array_append(gc->relation_schemas, schema);
	}

	return schema;
}

const char *GraphContext_GetNodeLabel(const GraphContext *gc, Node *n) {
	int label_id = Graph_GetNodeLabel(gc->g, ENTITY_GET_ID(n));
	if(label_id == GRAPH_NO_LABEL) return NULL;
	return gc->node_schemas[label_id]->name;
}

const char *GraphContext_GetEdgeRelationType(const GraphContext *gc, Edge *e) {
	int reltype_id = Graph_GetEdgeRelation(gc->g, e);
	assert(reltype_id != GRAPH_NO_RELATION);
	return gc->relation_schemas[reltype_id]->name;
}

uint GraphContext_AttributeCount(GraphContext *gc) {
	return raxSize(gc->attributes);
}

Attribute_ID GraphContext_FindOrAddAttribute(GraphContext *gc, const char *attribute) {
	// See if attribute already exists.
	Attribute_ID *pAttribute_id = NULL;
	Attribute_ID attribute_id = GraphContext_GetAttributeID(gc, attribute);

	if(attribute_id == ATTRIBUTE_NOTFOUND) {
		attribute_id = raxSize(gc->attributes);
		pAttribute_id = rm_malloc(sizeof(Attribute_ID));
		*pAttribute_id = attribute_id;

		raxInsert(gc->attributes,
				  (unsigned char *)attribute,
				  strlen(attribute),
				  pAttribute_id,
				  NULL);
		gc->string_mapping = array_append(gc->string_mapping, rm_strdup(attribute));
	}

	return attribute_id;
}

const char *GraphContext_GetAttributeString(const GraphContext *gc, Attribute_ID id) {
	assert(id < array_len(gc->string_mapping));
	return gc->string_mapping[id];
}

Attribute_ID GraphContext_GetAttributeID(const GraphContext *gc, const char *attribute) {
	Attribute_ID *id = raxFind(gc->attributes, (unsigned char *)attribute, strlen(attribute));
	if(id == raxNotFound) return ATTRIBUTE_NOTFOUND;
	return *id;
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
bool GraphContext_HasIndices(GraphContext *gc) {
	uint schema_count = array_len(gc->node_schemas);
	for(uint i = 0; i < schema_count; i++) {
		if(Schema_HasIndices(gc->node_schemas[i])) return true;
	}
	return false;
}

Index *GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *field,
							 IndexType type) {
	// Retrieve the schema for this label
	Schema *schema = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
	if(schema == NULL) return NULL;

	return Schema_GetIndex(schema, field, type);
}

int GraphContext_AddIndex(Index **idx, GraphContext *gc, const char *label, const char *field,
						  IndexType type) {
	assert(idx && gc && label && field);

	// Retrieve the schema for this label
	Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
	if(s == NULL) s = GraphContext_AddSchema(gc, label, SCHEMA_NODE);
	return Schema_AddIndex(idx, s, field, type);
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *field,
							 IndexType type) {
	// Retrieve the schema for this label
	Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
	if(s == NULL) return INDEX_FAIL;
	return Schema_RemoveIndex(s, field, type);
}

// Delete all references to a node from any indices built upon its properties
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, Node *n) {
	Schema *s = NULL;

	if(n->label) {
		// Node will have a label string if one was specified in the query MATCH clause
		s = GraphContext_GetSchema(gc, n->label, SCHEMA_NODE);
	} else {
		EntityID node_id = ENTITY_GET_ID(n);
		// Otherwise, look up the offset of the matching label (if any)
		int schema_id = Graph_GetNodeLabel(gc->g, node_id);
		// Do nothing if node had no label
		if(schema_id == GRAPH_NO_LABEL) return;
		s = GraphContext_GetSchemaByID(gc, schema_id, SCHEMA_NODE);
	}

	// Update any indices this entity is represented in
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx) Index_RemoveNode(idx, n);
	idx = Schema_GetIndex(s, NULL, IDX_EXACT_MATCH);
	if(idx) Index_RemoveNode(idx, n);
}

//------------------------------------------------------------------------------
// Functions for globally tracking GraphContexts
//------------------------------------------------------------------------------

// Register a new GraphContext for module-level tracking
void GraphContext_RegisterWithModule(GraphContext *gc) {
	graphs_in_keyspace = array_append(graphs_in_keyspace, gc);
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
// Free routine
//------------------------------------------------------------------------------

// Free all data associated with graph
static void _GraphContext_Free(void *arg) {
	GraphContext *gc = (GraphContext *)arg;
	uint len;
	rm_free(gc->graph_name);

	// Disable matrix synchronization for graph deletion.
	Graph_SetMatrixPolicy(gc->g, DISABLED);
	Graph_Free(gc->g);

	// Free all node schemas
	if(gc->node_schemas) {
		len = array_len(gc->node_schemas);
		for(uint32_t i = 0; i < len; i ++) {
			Schema_Free(gc->node_schemas[i]);
		}
		array_free(gc->node_schemas);
	}

	// Free all relation schemas
	if(gc->relation_schemas) {
		len = array_len(gc->relation_schemas);
		for(uint32_t i = 0; i < len; i ++) {
			Schema_Free(gc->relation_schemas[i]);
		}
		array_free(gc->relation_schemas);
	}

	// Free attribute mappings
	if(gc->attributes) raxFreeWithCallback(gc->attributes, rm_free);
	if(gc->string_mapping) {
		len = array_len(gc->string_mapping);
		for(uint32_t i = 0; i < len; i ++) {
			rm_free(gc->string_mapping[i]);
		}
		array_free(gc->string_mapping);
	}

	rm_free(gc);
}
