/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "../slow_log/slow_log.h"
#include "graph.h"
#include "../serializers/encode_context.h"
#include "../serializers/decode_context.h"
#include "../util/cache/cache.h"

// GraphContext holds refrences to various elements of a graph object
// It is the value sitting behind a Redis graph key
//
// the graph context is versioned, the version value itself is meaningless
// it is used as a "signature" for the graph schema: (labels, relationship-types
// and attribute set) client libraries which cache the mapping between graph
// schema elements and their internal IDs (see COMPACT reply formatter)
// can use the graph version to understand if the schema was modified
// and take action accordingly

typedef struct {
	Graph *g;                               // container for all matrices and entity properties
	int ref_count;                          // number of active references
	rax *attributes;                        // from strings to attribute IDs
	pthread_rwlock_t _attribute_rwlock;     // read-write lock to protect access to the attribute maps
	char *graph_name;                       // string associated with graph
	char **string_mapping;                  // from attribute IDs to strings
	Schema **node_schemas;                  // array of schemas for each node label
	Schema **relation_schemas;              // array of schemas for each relation type
	unsigned short index_count;             // number of indicies
	SlowLog *slowlog;                       // slowlog associated with graph
	GraphEncodeContext *encoding_context;   // encode context of the graph
	GraphDecodeContext *decoding_context;   // decode context of the graph
	Cache *cache;                           // global cache of execution plans
	XXH32_hash_t version;                   // graph version
} GraphContext;

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

// Creates and initializes a graph context struct.
GraphContext *GraphContext_New
(
	const char *graph_name,
	size_t node_cap,
	size_t edge_cap
);

// retrive the graph context according to the graph name
// readOnly is the access mode to the graph key
GraphContext *GraphContext_Retrieve
(
	RedisModuleCtx *ctx,
	RedisModuleString *graphID,
	bool readOnly,
	bool shouldCreate
);

// graphContext_Retrieve counterpart, releases a retrieved GraphContext
void GraphContext_Release
(
	GraphContext *gc
);

// mark graph key as "dirty" for Redis to pick up on
void GraphContext_MarkWriter
(
	RedisModuleCtx *ctx,
	GraphContext *gc
);

// mark graph as deleted, reduce graph reference count by 1
void GraphContext_Delete
(
	GraphContext *gc
);

// get graph name out of graph context
const char *GraphContext_GetName
(
	const GraphContext *gc
);

// rename a graph context
void GraphContext_Rename
(
	GraphContext *gc,
	const char *name
);

// Get graph context version
XXH32_hash_t GraphContext_GetVersion
(
	const GraphContext *gc
);

//------------------------------------------------------------------------------
// Schema API
//------------------------------------------------------------------------------

// retrieve number of schemas created for given type
unsigned short GraphContext_SchemaCount
(
	const GraphContext *gc,
	SchemaType t
);

// retrieve the specific schema for the provided ID
Schema *GraphContext_GetSchemaByID
(
	const GraphContext *gc,
	int id,
	SchemaType t
);

// retrieve the specific schema for the provided node label
// or relation type string
Schema *GraphContext_GetSchema
(
	const GraphContext *gc,
	const char *label,
	SchemaType t
);

// add a new schema and matrix for the given label
Schema *GraphContext_AddSchema
(
	GraphContext *gc,
	const char *label,
	SchemaType t
);

// retrieve the label string for a given Node object
const char *GraphContext_GetNodeLabel
(
	const GraphContext *gc,
	Node *n
);

// retrieve the relation type string for a given edge object
const char *GraphContext_GetEdgeRelationType
(
	const GraphContext *gc,
	Edge *e
);

// retrieve number of unique attribute keys
uint GraphContext_AttributeCount
(
	GraphContext *gc
);

// retrieve an attribute ID given a string, creating one if not found
Attribute_ID GraphContext_FindOrAddAttribute
(
	GraphContext *gc,
	const char *attribute
);

// retrieve an attribute string given an ID
const char *GraphContext_GetAttributeString
(
	GraphContext *gc,
	Attribute_ID id
);

// retrieve an attribute ID given a string
// or ATTRIBUTE_NOTFOUND if attribute doesn't exist
Attribute_ID GraphContext_GetAttributeID
(
	GraphContext *gc,
	const char *str
);

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------

bool GraphContext_HasIndices
(
	GraphContext *gc
);

// attempt to retrieve an index on the given label and attribute IDs
Index *GraphContext_GetIndexByID
(
	const GraphContext *gc,
	int id,
	Attribute_ID *attribute_id,
	IndexType type,
	SchemaType t
);

// attempt to retrieve an index on the given label and attribute
Index *GraphContext_GetIndex
(
	const GraphContext *gc,
	const char *label,
	Attribute_ID *attribute_id,
	IndexType type,
	SchemaType schema_type
);

// create an index for the given label and attribute
int GraphContext_AddIndex
(
	Index **idx,
	GraphContext *gc,
	SchemaType schema_type,
	const char *label,
	const char *field,
	IndexType index_type
);

// remove and free an index
int GraphContext_DeleteIndex
(
	GraphContext *gc,
	SchemaType schema_type,
	const char *label,
	const char *field,
	IndexType type
);

// remove a single node from all indices that refer to it
void GraphContext_DeleteNodeFromIndices
(
	GraphContext *gc,
	Node *n
);

// remove a single edge from all indices that refer to it
void GraphContext_DeleteEdgeFromIndices
(
	GraphContext *gc,
	Edge *e
);

// add GraphContext to global array
void GraphContext_RegisterWithModule
(
	GraphContext *gc
);

// retrive GraphContext from the global array, by name
// if no such graph is registered, NULL is returned
GraphContext *GraphContext_GetRegisteredGraphContext
(
	const char *graph_name
);

// remove GraphContext from global array
void GraphContext_RemoveFromRegistry
(
	GraphContext *gc
);

//------------------------------------------------------------------------------
// Slowlog API
//------------------------------------------------------------------------------

SlowLog *GraphContext_GetSlowLog
(
	const GraphContext *gc
);

//------------------------------------------------------------------------------
// Cache API
//------------------------------------------------------------------------------

// return cache associated with graph context and current thread id
Cache *GraphContext_GetCache
(
	const GraphContext *gc
);

