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

  unsigned int relation_cap;       // Capacity of relation LabelStore array
  unsigned int relation_count;     // Number of relation tyes
  unsigned int label_cap;          // Capacity of node LabelStore array
  unsigned int label_count;        // Number of label matrices.

  LabelStore *relation_allstore;   // Schema for all relation types
  LabelStore *node_allstore;       // Schema for all/unspecified node labels
  LabelStore **relation_stores;    // Array of schemas for each relation type
  LabelStore **node_stores;        // Array of schemas for each node label 

  unsigned int index_cap;          // Capacity of indices array
  unsigned int index_count;        // Number of indices
  Index **indices;                 // Array of all indices on label-property pairs

  pthread_rwlock_t _rwlock;        // Read-write lock scoped to this specific graph
  bool writelocked;                // true if the read-write lock was acquired by a writer
} GraphContext;

/* GraphContext synchronization functions
 * The GraphContext is initialized with a read-write lock allowing
 * concurrent access from one writer or N readers. */
// Acquire a lock that does not restrict access from additional reader threads
void GraphContext_AcquireReadLock(GraphContext *gc);
// Acquire a lock for exclusive access to this graph's data
void GraphContext_AcquireWriteLock(GraphContext *gc);
// Release the held lock
void GraphContext_ReleaseLock(GraphContext *gc);

/* GraphContext API */
GraphContext* GraphContext_New(RedisModuleCtx *ctx, RedisModuleString *rs_name);
GraphContext* GraphContext_Get(RedisModuleCtx *ctx, RedisModuleString *rs_graph_name, bool readonly);

/* LabelStore API */
// Get the offset of the given node label or relation type into the matrix and LabelStore arrays
int GraphContext_GetLabelID(const GraphContext *gc, const char *label, LabelStoreType t);
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
Index* GraphContext_GetIndex(GraphContext *gc, const char *label, const char *property);
// Create and populate an index for the given label and property
void GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property);
// Remove and free an index
int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property);

// Free the GraphContext and all associated graph data
void GraphContext_Free(GraphContext *gc);

#endif

