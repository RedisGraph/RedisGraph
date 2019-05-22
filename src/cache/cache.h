/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef CACHE_H
#define CACHE_H

#include "./cache_includes.h"
#include "./lru_cache_manager/lru_cache_manager.h"
#include "./rax_cache_storage/rax_cache_storage.h"
#include "xxhash_query_hash/xxhash_query_hash.h"

/**
 * A struct to for read queries cache storage. 
 * Holds a managment struct (LRU policy) and a RAX radix tree for storage.
 * A read-write lock is set for multi-threaded operatins.
 */
typedef struct Cache
{
    LRUCacheManager *lruCacheManager; // Cache Manager - LRU policy
    RaxCacheStorage *raxCacheStorage; // Storage - Rax radix tree
    pthread_rwlock_t rwlock;          // Read-Write lock
    bool isValid;
} Cache;


/**
 * @brief  Initilize a  cache
 * @param  cacheSize: Cache size (in entries)
 * @retval cache pointer - Initialized empty cache
 */
Cache *Cache_New(size_t cacheSize, cacheValueFreeFunc freeCB);

/**
 * @brief  Destory a  cache
 * @param  *cache: cache pointer
 * @retval None
 */
void Cache_Free(Cache *cache);

/**
 * @brief  Returns a value if it is cached, NULL otherwise  
 * @param  *cache: cache pointer
 * @param  *query: Cypher query - charecters array
 * @retval  pointer with the cached answer, NULL if the query isn't cached
 */
void *getCacheValue(Cache *cache, const char *query, size_t queryLength);

/**
 * @brief  Sotres a query in the cache
 * @param  *cache: cache pointer
 * @param  *query: Cypher query to be associated with the value- charecters array
 * @param  queryLength:
 * @param  *value: pointer with the relevant value
 * @retval None
 */
void storeCacheValue(Cache *cache, const char *query, size_t queryLength, void *value);

/**
 * @brief  Removes a value from the cache
 * @param  *cache: cache pointer
 * @param  *query: Cypher query to be associated with the value- charecters array
 * @param  queryLength:
 * @retval None
 */
void removeCacheValue(Cache *cache, const char *query, size_t queryLength);

/**
 * @brief  Clears the cache from entries
 * @param  *cache: cache pointer
 * @retval None
 */
void clearCache(Cache *cache);

void markCacheInvalid(Cache *cache);

#endif