#include "graphcontext.h"
#include "serializers/graphcontext_type.h"
#include "../util/rmalloc.h"

//------------------------------------------------------------------------------
// Private functions
//------------------------------------------------------------------------------
// This is a pretty weak approach to index retrieval - revisit later.
int _GraphContext_IndexOffset(const GraphContext *gc, const char *label, const char *property) {
  Index *idx;
  for (int i = 0; i < gc->index_count; i ++) {
    idx = gc->indices[i];
    if (!strcmp(label, idx->label) && !strcmp(property, idx->property)) return i;
  }
  return -1;
}

int _GraphContext_GetLabelID(const GraphContext *gc, const char *label, LabelStoreType t) {
  LabelStore **labels;
  size_t count;
  if (t == STORE_NODE) {
    labels = gc->node_stores;
    count = gc->label_count;
  } else {
    labels = gc->relation_stores;
    count = gc->relation_count;
  }

  // TODO optimize lookup
  for (int i = 0; i < count; i ++) {
    if (!strcmp(label, labels[i]->label)) return i;
  }
  return GRAPH_NO_LABEL; // equivalent to GRAPH_NO_RELATION
}

//------------------------------------------------------------------------------
// Read/Write lock
//------------------------------------------------------------------------------

void GraphContext_AcquireReadLock(GraphContext *gc) {
    pthread_rwlock_rdlock(&gc->_rwlock);
}

void GraphContext_AcquireWriteLock(GraphContext *gc) {
    pthread_rwlock_wrlock(&gc->_rwlock);
    gc->writelocked = true;
}

void GraphContext_ReleaseLock(GraphContext *gc) {
    gc->writelocked = false;
    pthread_rwlock_unlock(&gc->_rwlock);
}

//------------------------------------------------------------------------------
// GraphContext API
//------------------------------------------------------------------------------

GraphContext* GraphContext_New(RedisModuleCtx *ctx, RedisModuleString *rs_name) {
  // Create key for GraphContext from the unmodified string provided by the user
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    // Return NULL if key already exists
    RedisModule_CloseKey(key);
    return NULL;
  }

  GraphContext *gc = rm_malloc(sizeof(GraphContext));

  // Initialize a read-write lock scoped to the individual graph and its data
  assert(pthread_rwlock_init(&gc->_rwlock, NULL) == 0);

  // GraphContext_New can only be invoked from writing contexts
  GraphContext_AcquireWriteLock(gc);
  gc->writelocked = true;

  gc->relation_cap = GRAPH_DEFAULT_RELATION_CAP;
  gc->relation_count = 0;
  gc->label_cap = GRAPH_DEFAULT_LABEL_CAP;
  gc->label_count = 0;
  gc->index_cap = DEFAULT_INDEX_CAP;
  gc->index_count = 0;

  // Initialize the graph's matrices and datablock storage
  gc->g = Graph_New(GRAPH_DEFAULT_NODE_CAP);

  gc->graph_name = rm_strdup(RedisModule_StringPtrLen(rs_name, NULL));
  // Allocate the default space for stores and indices
  gc->node_stores = rm_malloc(gc->label_cap * sizeof(LabelStore*));
  gc->relation_stores = rm_malloc(gc->relation_cap * sizeof(LabelStore*));
  gc->indices = rm_malloc(gc->index_cap * sizeof(Index*));

  // Initialize the generic label and relation stores
  gc->node_allstore = LabelStore_New("ALL", GRAPH_NO_LABEL);
  gc->relation_allstore = LabelStore_New("ALL", GRAPH_NO_RELATION);

  // Set and close GraphContext key in Redis keyspace
  RedisModule_ModuleTypeSetValue(key, GraphContextRedisModuleType, gc);
  RedisModule_CloseKey(key);

  return gc;
}

GraphContext* GraphContext_Get(RedisModuleCtx *ctx, RedisModuleString *rs_name, bool readonly) {
  RedisModuleKey *key = RedisModule_OpenKey(ctx, rs_name, REDISMODULE_WRITE);
  if (RedisModule_ModuleTypeGetType(key) != GraphContextRedisModuleType) {
    RedisModule_CloseKey(key);
    return NULL;
  }
  GraphContext *gc = RedisModule_ModuleTypeGetValue(key);
  RedisModule_CloseKey(key);

  // Acquire the appropriate read-write lock
  if (readonly) {
    GraphContext_AcquireReadLock(gc);
  } else {
    GraphContext_AcquireWriteLock(gc);
  }

  return gc;
}

//------------------------------------------------------------------------------
// Matrix API
//------------------------------------------------------------------------------
GrB_Matrix GraphContext_GetMatrix(const GraphContext *gc, const char *label, LabelStoreType t) {
  int id = _GraphContext_GetLabelID(gc, label, t);
  if (id == GRAPH_NO_LABEL) return NULL;
  return Graph_GetLabel(gc->g, id, gc->writelocked);
}

//------------------------------------------------------------------------------
// LabelStore API
//------------------------------------------------------------------------------
LabelStore* GraphContext_AllStore(const GraphContext *gc, LabelStoreType t) {
  if (t == STORE_NODE) return gc->node_allstore;
  return gc->relation_allstore;
}

LabelStore* GraphContext_GetStore(const GraphContext *gc, const char *label, LabelStoreType t) {
  LabelStore **stores = (t == STORE_NODE) ? gc->node_stores : gc->relation_stores;

  int id = _GraphContext_GetLabelID(gc, label, t);
  if (id == GRAPH_NO_LABEL) return NULL;

  return stores[id];
}

// The next two functions are identical save for whether their target is a label or relation type,
// but modify enough variables to make consolidating them unwieldy.
LabelStore* GraphContext_AddLabel(GraphContext *gc, const char *label) {
  if(gc->label_count == gc->label_cap) {
    gc->label_cap += 4;
    // Add space for additional label stores.
    gc->node_stores = rm_realloc(gc->node_stores, gc->label_cap * sizeof(LabelStore*));
    // Add space for additional label matrices.
    gc->g->labels = rm_realloc(gc->g->labels, gc->label_cap * sizeof(GrB_Matrix));

  }
  Graph_AddLabel(gc->g);
  LabelStore *store = LabelStore_New(label, gc->label_count);
  gc->node_stores[gc->label_count] = store;
  gc->label_count++;

  return store;
}

LabelStore* GraphContext_AddRelationType(GraphContext *gc, const char *label) {
  if(gc->relation_count == gc->relation_cap) {
    gc->relation_cap += 4;
    gc->relation_stores = rm_realloc(gc->relation_stores, gc->relation_cap * sizeof(LabelStore*));
    gc->g->relations = rm_realloc(gc->g->relations, gc->relation_cap * sizeof(GrB_Matrix));
  }
  Graph_AddRelationType(gc->g);

  LabelStore *store = LabelStore_New(label, gc->relation_count);
  gc->relation_stores[gc->relation_count] = store;
  gc->relation_count++;

  return store;
}

//------------------------------------------------------------------------------
// Index API
//------------------------------------------------------------------------------
bool GraphContext_HasIndices(GraphContext *gc) {
  return gc->index_count > 0;
}

Index* GraphContext_GetIndex(const GraphContext *gc, const char *label, const char *property) {
  int offset = _GraphContext_IndexOffset(gc, label, property);
  if (offset < 0) return INDEX_FAIL;

  return gc->indices[offset];
}

void GraphContext_AddIndex(GraphContext *gc, const char *label, const char *property) {
  if(gc->index_count == gc->index_cap) {
    gc->index_cap += 4;
    gc->indices = rm_realloc(gc->indices, gc->index_cap * sizeof(Index*));
  }

  // Generate an iterator over the specified label
  const GrB_Matrix label_matrix = GraphContext_GetMatrix(gc, label, STORE_NODE);
  TuplesIter *it = TuplesIter_new(label_matrix);
  // Populate an index by using this iterator to seek into the data block.
  Index *idx = Index_Create(gc->g->nodes, it, label, property);
  TuplesIter_free(it);
  gc->indices[gc->index_count] = idx;
  gc->index_count++;
}

int GraphContext_DeleteIndex(GraphContext *gc, const char *label, const char *property) {
  int offset = _GraphContext_IndexOffset(gc, label, property);
  if (offset < 0) return INDEX_FAIL;

  Index *idx = gc->indices[offset];
  Index_Free(idx);
  gc->index_count --;
  // If the deleted index was not the last, swap the last into the newly-emptied position
  if (offset < gc->index_count) gc->indices[offset] = gc->indices[gc->index_count];

  return INDEX_OK;
}

// Free all data associated with graph
void GraphContext_Free(GraphContext *gc) {
  Graph_Free(gc->g);
  rm_free(gc->graph_name);

  // Free generic stores
  LabelStore_Free(gc->node_allstore);
  LabelStore_Free(gc->relation_allstore);

  // Free all node stores
  for (int i = 0; i < gc->label_count; i ++) {
    LabelStore_Free(gc->node_stores[i]);
  }
  rm_free(gc->node_stores);

  // Free all relation stores
  for (int i = 0; i < gc->relation_count; i ++) {
    LabelStore_Free(gc->relation_stores[i]);
  }
  rm_free(gc->relation_stores);

  // Free all indices
  for (int i = 0; i < gc->index_count; i ++) {
    Index_Free(gc->indices[i]);
  }
  rm_free(gc->indices);

  GraphContext_ReleaseLock(gc);
  // Destroy read-write lock
  pthread_rwlock_destroy(&gc->_rwlock);

  rm_free(gc);
}

