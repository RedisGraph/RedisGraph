/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef RESULTSET_CACHE_H
#define RESULTSET_CACHE_H
#include <pthread.h>
#include "result_set_cache_includes.h"
#include "./lru_cache_manager/lru_cache_manager.h"
#include "./rax_cache_storage/rax_cache_storage.h"
#include "./xxhash_query_hash/xxhash_query_hash.h"

/**
 * A struct to for read queries cache storage. 
 * Holds a managment struct (LRU policy) and a RAX radix tree for storage.
 * A read-write lock is set for multi-threaded operatins.
 */
typedef struct ResultSetCache{
    LRUCacheManager *lruCacheManager;       // Cache Manager - LRU policy
    RaxCacheStorage *raxCacheStorage;       // Storage - Rax radix tree
    pthread_rwlock_t rwlock;                // Read-Write lock
} ResultSetCache;

/**
 * @brief  Initilize a result-sets cache
 * @param  cacheSize: Cache size (in entries)
 * @retval ResultSetCache pointer - Initialized empty cache
 */
ResultSetCache *ResultSetCache_New(size_t cacheSize);

/**
 * @brief  Destory a results-sets cache
 * @param  *resultSetCache: ResultSetCache pointer
 * @retval None
 */
void ResultSetCache_Free(ResultSetCache *resultSetCache);

/**
 * @brief  Returns a ResultSet if it is cached, NULL otherwise  
 * @param  *resultSetCache: ResultSetCache pointer
 * @param  *query: Cypher query - charecters array
 * @retval ResultSet pointer with the cached answer, NULL if the query isn't cached
 */
ResultSet *getResultSet(ResultSetCache* resultSetCache, const char *query);

/**
 * @brief  Sotres a query in the cache
 * @param  *resultSetCache: ResultSetCache pointer
 * @param  *query: Cypher query to be associated with the result set- charecters array
 * @param  *resultSet: ResultSet pointer with the relevant result
 * @retval None
 */
void storeResultSet(ResultSetCache *resultSetCache, const char *query, ResultSet *resultSet);

/**
 * @brief  Clears the cache from entries
 * @param  *resultSetCache: ResultSetCache pointer
 * @retval None
 */
void clearCache(ResultSetCache *resultSetCache);

#endif