#ifndef GRAPHCONTEXT_H
#define GRAPHCONTEXT_H

#include <assert.h>
#include "../redismodule.h"
#include "../index/index.h"
#include "../stores/store.h"
#include "graph.h"

#define DEFAULT_INDEX_CAP 4

typedef struct {
  char *graph_name;                // String associated with graph
  Graph *g;                        // Container for all matrices and entity properties

  LabelStore *relation_allstore;   // Schema for all relation types
  LabelStore *node_allstore;       // Schema for all/unspecified node labels
  LabelStore **relation_stores;    // Array of schemas for each relation type
  LabelStore **node_stores;        // Array of schemas for each node label 

  Index **indices;                 // Array of all indices on label-property pairs
} GraphContext;

/* GraphContext API */
GraphContext* GraphContext_New(RedisModuleCtx *ctx, const char *graphname,
                               size_t node_cap, size_t edge_cap);
GraphContext* GraphContext_Retrieve(RedisModuleCtx *ctx, const char *graphname);

// Retrives graph context from thread local storage.
GraphContext* GraphContext_GetFromLTS();

/* LabelStore API */
// Retrieve the generic store for node labels or relation types
LabelStore* GraphContext_AllStore(const GraphContext *gc, LabelStoreType t);
// Retrieve the specific store for the provided node label or relation type string
LabelStore* GraphContext_GetStore(const GraphContext *gc, const char *label, LabelStoreType t);
// Add a new store and matrix for the given node label
LabelStore* GraphContext_AddLabel(GraphContext *gc, const char *label);
// Add a new store and matrix for the given relation type 
LabelStore* GraphContext_AddRelationType(GraphContext *gc, const char *label);

/* Index API */
bool GraphContext_HasIndices(GraphContext *gc);
// Attempt to retrieve an index on the given label and property
Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *property);
// Create and populate an index for the given label and property
int GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property);
// Remove and free an index
int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property);

// Add a single node to all indices its properties match
void GraphContext_AddNodeToIndices(GraphContext *gc, LabelStore *store, Node *n);
// Remove a single node from all indices that refer to it
void GraphContext_DeleteNodeFromIndices(GraphContext *gc, LabelStore *store, Node *n);
// Remove a single node property from an index and re-insert with new value
void GraphContext_UpdateNodeIndices(GraphContext *gc, LabelStore *store, NodeID id, EntityProperty *prop, SIValue *newval);

// Free the GraphContext and all associated graph data
void GraphContext_Free(GraphContext *gc);

#endif

