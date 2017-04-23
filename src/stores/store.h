#ifndef __STORE_H__
#define __STORE_H__

#include "../redismodule.h"
#include "../util/triemap/triemap.h"

#define STORE_PREFIX "redis_graph_store"
#define LABEL_DEFAULT_SCORE 0

typedef enum {
  STORE_NODE,
  STORE_EDGE,
} StoreType;

typedef TrieMap Store;
typedef TrieMapIterator StoreIterator;

Store *GetStore(RedisModuleCtx *ctx, StoreType type, const RedisModuleString *graph, const RedisModuleString* label);

void Store_Insert(Store *store, const RedisModuleString *id);

void Store_Remove(Store *store, const RedisModuleString *id);

StoreIterator *Store_Search(Store *store, const char *prefix);

// Returns the next id from the cursor.
char *StoreIterator_Next(StoreIterator *cursor);

void StoreIterator_Free(StoreIterator* iterator);

#endif