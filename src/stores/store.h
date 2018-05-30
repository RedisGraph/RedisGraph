#ifndef __LABEL_STORE_H__
#define __LABEL_STORE_H__

#include "../redismodule.h"
#include "../graph/graph_entity.h"
#include "../util/triemap/triemap.h"

#define LABELSTORE_PREFIX "redis_graph_store"

typedef enum {
  STORE_NODE,
  STORE_EDGE,
} LabelStoreType;

typedef struct {
  int id;               /* Internal ID to either vector or matrix within the graph. */
  char *label;
  TrieMap *properties;  /* Keeps track on label schema. */
} LabelStore;

// Creates a new label store.
LabelStore *LabelStore_New(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label, int id);

/* Get a label store. */
LabelStore *LabelStore_Get(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label);

/* Get all stores of given type. */
void LabelStore_Get_ALL(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, LabelStore **stores, size_t *stores_len);

/* Update store schema with given properties. */
void LabelStore_UpdateSchema(LabelStore *store, int prop_count, char **properties);

/* Free store. */
void LabelStore_Free(LabelStore *store, void (*freeCB)(void *));

#endif /* __LABEL_STORE_H__ */