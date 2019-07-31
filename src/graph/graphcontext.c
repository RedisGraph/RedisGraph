/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <sys/param.h>
#include "graphcontext.h"
#include "serializers/graphcontext_type.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"
#include "../redismodule.h"

extern pthread_key_t _tlsGCKey;    // Thread local storage graph context key.

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

GraphContext *GraphContext_New(RedisModuleCtx *ctx, const char *graphname,
							   size_t node_cap, size_t edge_cap) {
	GraphContext *gc = NULL;

	// Create key for GraphContext from the unmodified string provided by the user
	RedisModuleString *rs_name = RedisModule_CreateString(ctx, graphname, strlen(graphname));
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
	if(RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
		goto cleanup;
	}

	gc = rm_malloc(sizeof(GraphContext));

	// No indicies.
	gc->index_count = 0;

	// Initialize the graph's matrices and datablock storage
	gc->g = Graph_New(node_cap, edge_cap);
	gc->graph_name = rm_strdup(graphname);
	// Allocate the default space for schemas and indices
	gc->node_schemas = array_new(Schema *, GRAPH_DEFAULT_LABEL_CAP);
	gc->relation_schemas = array_new(Schema *, GRAPH_DEFAULT_RELATION_TYPE_CAP);

	gc->string_mapping = array_new(char *, 64);
	gc->attributes = NewTrieMap();

	assert(pthread_setspecific(_tlsGCKey, gc) == 0);

	// Set and close GraphContext key in Redis keyspace
	RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);

cleanup:
	RedisModule_CloseKey(key);
	RedisModule_FreeString(ctx, rs_name);
	return gc;
}

GraphContext *GraphContext_Retrieve(RedisModuleCtx *ctx, const char *graphname, bool readOnly) {
	GraphContext *gc = NULL;
	RedisModuleString *rs_name = RedisModule_CreateString(ctx, graphname, strlen(graphname));
	int readWriteFlag = readOnly ? REDISMODULE_READ : REDISMODULE_WRITE;
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, readWriteFlag);
	if(RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
		goto cleanup;
	}
	gc = RedisModule_ModuleTypeGetValue(key);

	// Force GraphBLAS updates and resize matrices to node count by default
	Graph_SetMatrixPolicy(gc->g, SYNC_AND_MINIMIZE_SPACE);

	assert(pthread_setspecific(_tlsGCKey, gc) == 0);

cleanup:
	RedisModule_FreeString(ctx, rs_name);
	RedisModule_CloseKey(key);
	return gc;
}

GraphContext *GraphContext_GetFromTLS() {
	GraphContext *gc = pthread_getspecific(_tlsGCKey);
	assert(gc);
	return gc;
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
	return gc->attributes->cardinality;
}

Attribute_ID GraphContext_FindOrAddAttribute(GraphContext *gc, const char *attribute) {
	// See if attribute already exists.
	Attribute_ID *pAttribute_id = NULL;
	Attribute_ID attribute_id = GraphContext_GetAttributeID(gc, attribute);

	if(attribute_id == ATTRIBUTE_NOTFOUND) {
		attribute_id = gc->attributes->cardinality;
		pAttribute_id = rm_malloc(sizeof(Attribute_ID));
		*pAttribute_id = attribute_id;

		TrieMap_Add(gc->attributes,
					(char *)attribute,
					strlen(attribute),
					pAttribute_id,
					TrieMap_DONT_CARE_REPLACE);
		gc->string_mapping = array_append(gc->string_mapping, rm_strdup(attribute));
	}

	return attribute_id;
}

const char *GraphContext_GetAttributeString(const GraphContext *gc, Attribute_ID id) {
	assert(id < array_len(gc->string_mapping));
	return gc->string_mapping[id];
}

Attribute_ID GraphContext_GetAttributeID(const GraphContext *gc, const char *attribute) {
	Attribute_ID *id = TrieMap_Find(gc->attributes, (char *)attribute, strlen(attribute));
	if(id == TRIEMAP_NOTFOUND) return ATTRIBUTE_NOTFOUND;
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
// Free routine
//------------------------------------------------------------------------------

// Free all data associated with graph
void GraphContext_Free(GraphContext *gc) {
	Graph_Free(gc->g);
	rm_free(gc->graph_name);

	uint len;

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
	if(gc->attributes) TrieMap_Free(gc->attributes, rm_free);
	if(gc->string_mapping) {
		len = array_len(gc->string_mapping);
		for(uint32_t i = 0; i < len; i ++) {
			rm_free(gc->string_mapping[i]);
		}
		array_free(gc->string_mapping);
	}

	rm_free(gc);
}
