/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

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
  int id;               /* Internal ID to a matrix within the graph. */
  char *label;
  TrieMap *properties;  /* Keeps track after label schema. */
} LabelStore;

// Creates a new label store.
LabelStore *LabelStore_New(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label, int id);

/* Assign an ID to a label-property pair.
 * Label store schema values are currently size_t pointers that represent the ID of the index on that property
 * (if any). That usage is not defined within the LabelStore, however. */
void LabelStore_AssignValue(LabelStore *store, char *property, size_t *id);

/* Get a label store. */
LabelStore *LabelStore_Get(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label);

/* Update store schema with given properties. */
void LabelStore_UpdateSchema(LabelStore *store, int prop_count, char **properties);

/* Returns every Redis key name holding a label store object, for given graph. */
RedisModuleString **LabelStore_GetKeys(RedisModuleCtx *ctx, const char *graphID, size_t *keyCount);

/* Free store. */
void LabelStore_Free(LabelStore *store);

#endif /* __LABEL_STORE_H__ */
