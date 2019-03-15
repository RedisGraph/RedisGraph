/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GRAPHCONTEXT_H
#define GRAPHCONTEXT_H

#include <assert.h>
#include "../redismodule.h"
#include "../index/index.h"
#include "../schema/schema.h"
#include "graph.h"

typedef struct {
  char *graph_name;                 // String associated with graph
  Graph *g;                         // Container for all matrices and entity properties

  TrieMap *attributes;              // From strings to attribute IDs
  char **string_mapping;            // From attribute IDs to strings

  Schema **relation_schemas;        // Array of schemas for each relation type
  Schema **node_schemas;            // Array of schemas for each node label 

  unsigned short index_count;       // Number of indicies.
} GraphContext;

/* GraphContext API */
GraphContext* GraphContext_New(RedisModuleCtx *ctx, const char *graphname,
                               size_t node_cap, size_t edge_cap);
GraphContext* GraphContext_Retrieve(RedisModuleCtx *ctx, const char *graphname);

// Retrives graph context from thread local storage.
GraphContext* GraphContext_GetFromTLS();

/* Schema API */
// Retrieve number of schemas created for given type.
unsigned short GraphContext_SchemaCount(const GraphContext *gc, SchemaType t);
// Retrieve the specific schema for the provided node label or relation type string
Schema* GraphContext_GetSchema(const GraphContext *gc, const char *label, SchemaType t);
// Retrieve the specific schema for the provided id.
Schema* GraphContext_GetSchemaByID(const GraphContext *gc, int id, SchemaType t);
// Add a new schema and matrix for the given label
Schema* GraphContext_AddSchema(GraphContext *gc, const char *label, SchemaType t);

// TODO needed?
// Add a new store and matrix for the given relation type
Schema* GraphContext_AddRelationType(GraphContext *gc, const char *label);
// Retrieve the label string for a given Node object
const char* GraphContext_GetNodeLabel(const GraphContext *gc, Node *n);
// Retrieve the relation type string for a given Edge object
const char* GraphContext_GetEdgeRelationType(const GraphContext *gc, Edge *e);

// Retrieve number of unique attribute keys
uint GraphContext_AttributeCount(GraphContext *gc);

// Adds an attribute string to GraphContext ID mappings
Attribute_ID GraphContext_AddAttribute(GraphContext *gc, const char *attribute);
// Attribute_ID GraphContext_GetAttributeID(GraphContext *gc, const char *attribute);
const char* GraphContext_GetAttributeString(const GraphContext *gc, Attribute_ID id);

/* Index API */
bool GraphContext_HasIndices(GraphContext *gc);
// Attempt to retrieve an index on the given label and attribute
Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *attribute);
// Create and populate an index for the given label and attribute
int GraphContext_AddIndex(GraphContext *gc, const char *label, const char *attribute);
// Remove and free an index
int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *attribute);

// Add a single node to all indices its properties match
void GraphContext_AddNodeToIndices(GraphContext *gc, Schema *s, Node *n);
// Remove a single node from all indices that refer to it
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, Node *n);

// Free the GraphContext and all associated graph data
void GraphContext_Free(GraphContext *gc);

#endif
