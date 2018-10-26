#ifndef _GRAPHCONTEXT_H
#define _GRAPHCONTEXT_H

#include <assert.h>
#include "../redismodule.h"
#include "../index/index.h"
#include "../stores/store.h"
#include "graph.h"

typedef struct {
  Graph *g;
  char *graph_name;

  unsigned int relation_cap;       // Current allocation size for relation type storage.
  unsigned int relation_count;     // Number of relation matrices.
  unsigned int label_cap;          // Current allocation size for node label storage.
  unsigned int label_count;        // Number of label matrices.

  Index **indices;
  unsigned int index_count;
  unsigned int index_cap;

  LabelStore *relation_allstore;
  LabelStore *node_allstore;
  LabelStore **relation_stores;
  LabelStore **node_stores;

  // Synchronization members:
  /* Read-write lock to allow one writer or N readers to concurrently
   * access this graph database. */
  pthread_rwlock_t _rwlock;
  // _writelocked is true if the read-write lock was acquired by a writer
  bool _writelocked;
} GraphContext;

// Synchronization functions
void GraphContext_AcquireReadLock(GraphContext *gc);
void GraphContext_AcquireWriteLock(GraphContext *gc);
void GraphContext_ReleaseLock(GraphContext *gc);

RedisModuleKey* GraphContext_Key(RedisModuleCtx *ctx, const char *graph_name);

GraphContext* GraphContext_New(RedisModuleCtx *ctx, RedisModuleString *rs_name);

GraphContext* GraphContext_Get(RedisModuleCtx *ctx, RedisModuleString *rs_graph_name, bool readonly);

LabelStore* GraphContext_AddLabel(GraphContext *gc, const char *label);
LabelStore* GraphContext_AddRelationType(GraphContext *gc, const char *label);

int GraphContext_GetLabelID(const GraphContext *gc, const char *label, LabelStoreType t);

bool GraphContext_HasIndices(GraphContext *gc);
Index* GraphContext_GetIndex(GraphContext *gc, const char *label, const char *property);
void GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property);
int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property);

LabelStore* GraphContext_AllStore(const GraphContext *gc, LabelStoreType t);
LabelStore* GraphContext_GetStore(const GraphContext *gc, const char *label, LabelStoreType t);

void GraphContext_Free(GraphContext *gc);

#endif

