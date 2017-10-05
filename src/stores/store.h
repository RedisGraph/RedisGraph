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

int Store_ID(char **id, StoreType type, const char *graph, const char *label);

Store *GetStore(RedisModuleCtx *ctx, StoreType type, const char *graph, const char* label);

// Returns the number of items within the store
int Store_Cardinality(Store *store);

void Store_Insert(Store *store, char *id, void *value);

int Store_Remove(Store *store, char *id, void (*freeCB)(void *));

StoreIterator *Store_Search(Store *store, const char *prefix);

void *Store_Get(Store *store, char *id);

void Store_Free(Store *store, void (*freeCB)(void *));

// Returns the next id from the cursor.
int StoreIterator_Next(StoreIterator *cursor, char **key, tm_len_t *len, void **value);

void StoreIterator_Free(StoreIterator* iterator);

#endif