#ifndef RESULTSET_CACHE_H
#define RESULTSET_CACHE_H

#include "result_set_cache_includes.h"
#include "./lru_cache_manager/lru_cache_manager.h"
#include "./rax_cache_storage/rax_cache_storage.h"
#include "./xxhash_query_hash/xxhash_query_hash.h"

typedef struct ResultSetCache{
    LRUCacheManager *lruCacheManager;
    RaxCacheStorage *raxCacheStorage;
} ResultSetCache;

ResultSet *getResultSet(ResultSetCache* resultSetCache, const char *query);
void storeResultSet(ResultSetCache *resultSetCache, const char *query, ResultSet *resultSet);

#endif