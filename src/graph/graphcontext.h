/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPHCONTEXT_H
#define GRAPHCONTEXT_H

#include <assert.h>
#include "../redismodule.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "../slow_log/slow_log.h"
#include "graph.h"
#include "../serializers/encode_context.h"
#include "../serializers/decode_context.h"
#include "../util/cache/cache.h"

typedef struct {
	Graph *g;                               // Container for all matrices and entity properties
	int ref_count;                          // Number of active references.
	rax *attributes;                        // From strings to attribute IDs
	pthread_rwlock_t _attribute_rwlock;     // Read-write lock to protect access to the attribute maps.
	char *graph_name;                       // String associated with graph
	char **string_mapping;                  // From attribute IDs to strings
	Schema **node_schemas;                  // Array of schemas for each node label
	Schema **relation_schemas;              // Array of schemas for each relation type
	unsigned short index_count;             // Number of indicies.
	SlowLog *slowlog;                       // Slowlog associated with graph.
	GraphEncodeContext *encoding_context;   // Encode context of the graph.
	GraphDecodeContext *decoding_context;   // Decode context of the graph.
	Cache **cache_pool;                     // Pool of execution plan caches, one per thread.
} GraphContext;

/* GraphContext API */
// Creates and initializes a graph context struct.
GraphContext *GraphContext_New(const char *graph_name, size_t node_cap, size_t edge_cap);
/* Retrive the graph context according to the graph name
 * readOnly is the access mode to the graph key */
GraphContext *GraphContext_Retrieve(RedisModuleCtx *ctx, RedisModuleString *graphID, bool readOnly,
									bool shouldCreate);
// GraphContext_Retrieve counterpart, releases a retrieved GraphContext.
void GraphContext_Release(GraphContext *gc);
// Mark graph key as "dirty" for Redis to pick up on.
void GraphContext_MarkWriter(RedisModuleCtx *ctx, GraphContext *gc);

// Mark graph as deleted, reduce graph reference count by 1.
void GraphContext_Delete(GraphContext *gc);

/* Schema API */
// Retrieve number of schemas created for given type.
unsigned short GraphContext_SchemaCount(const GraphContext *gc, SchemaType t);
// Retrieve the specific schema for the provided ID
Schema *GraphContext_GetSchemaByID(const GraphContext *gc, int id, SchemaType t);
// Retrieve the specific schema for the provided node label or relation type string
Schema *GraphContext_GetSchema(const GraphContext *gc, const char *label, SchemaType t);
// Add a new schema and matrix for the given label
Schema *GraphContext_AddSchema(GraphContext *gc, const char *label, SchemaType t);
// Retrieve the label string for a given Node object
const char *GraphContext_GetNodeLabel(const GraphContext *gc, Node *n);
// Retrieve the relation type string for a given Edge object
const char *GraphContext_GetEdgeRelationType(const GraphContext *gc, Edge *e);
// Retrieve number of unique attribute keys
uint GraphContext_AttributeCount(GraphContext *gc);
// Retrieve an attribute ID given a string, creating one if not found
Attribute_ID GraphContext_FindOrAddAttribute(GraphContext *gc, const char *attribute);
// Retrieve an attribute string given an ID
const char *GraphContext_GetAttributeString(const GraphContext *gc, Attribute_ID id);
// Retrieve an attribute ID given a string, or ATTRIBUTE_NOTFOUND if attribute doesn't exist.
Attribute_ID GraphContext_GetAttributeID(GraphContext *gc, const char *str);

/* Index API */
bool GraphContext_HasIndices(GraphContext *gc);
// Attempt to retrieve an index on the given label and attribute
Index *GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *field,
							 IndexType type);
// Create an index for the given label and attribute
int GraphContext_AddIndex(Index **idx, GraphContext *gc, const char *label, const char *field,
						  IndexType type);
// Remove and free an index
int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *field,
							 IndexType type);
// Remove a single node from all indices that refer to it
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, Node *n);

// Add GraphContext to global array
void GraphContext_RegisterWithModule(GraphContext *gc);

// Retrive GraphContext from the global array, by name. If no such graph is registered, NULL is returned.
GraphContext *GraphContext_GetRegisteredGraphContext(const char *graph_name);

// Remove GraphContext from global array
void GraphContext_RemoveFromRegistry(GraphContext *gc);

// Rename a graph context.
void GraphContext_Rename(GraphContext *gc, const char *name);

/* Slowlog API */
SlowLog *GraphContext_GetSlowLog(const GraphContext *gc);

/* Cache API - Return cache associated with graph context and current thread id. */
Cache *GraphContext_GetCache(const GraphContext *gc);

#endif

