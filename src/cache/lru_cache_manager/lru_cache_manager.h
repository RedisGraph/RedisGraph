/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef LRU_CACHE_MANAGER_H
#define LRU_CACHE_MANAGER_H
#include "../cache_includes.h"
#include "./lru_queue.h"

/**
 * @brief  Struct for manage the state of cache data, using LRU
 * @retval None
 */
typedef struct LRUCacheManager
{
  LRUQueue *queue;    // LRU Queue
} LRUCacheManager;

/**
 * @brief  Creates a LRU Cache manager, with initlized LRU queue of the given cacpaicty
 * @param  capacity: Cache cacpcity
 * @retval Pointer to the cache manager
 */
LRUCacheManager *LRUCacheManager_New(size_t capacity, cacheValueFreeFunc freeCB);

/**
 * @brief  Enqueue a new cache object in the cache
 * @note   Enqueuing is not an atomic procedure: 
 *         You first need to check if the cache is full, and if so, evict an entry
 * @param  *lruCacheManager: Cache manager address (pointer)
 * @param  *hashKey: cache entry key (charected arrau in size of HASH_KEY_LENGTH)
 * @param  *cacheValue: cache entry value 
 * @retval 
 */
CacheData *addToCache(LRUCacheManager *lruCacheManager, unsigned long long const hashKey, void *cacheValue);

/**
 * @brief  Evicts the least recently used entry from the cache queue
 * @param  *lruCacheManager: Cache manager address (pointer)
 * @retval Cache emtry which has bean removed
 */
CacheData *evictFromCache(LRUCacheManager *lruCacheManager);

/**
 * @brief  Increase the importance of cache entry (hot entry)
 * @param  *cacheManager: Cache manager address (pointer)
 * @param  *cacheData: Cache entry address (pointer)
 * @retval None
 */
void increaseImportance(LRUCacheManager *cacheManager, CacheData *cacheData);

/**
 * @brief  Removes a specific entry from the cache
 * @param  *cacheManager: Cache manager address (pointer)
 * @param  *cacheData: Cache entry address (pointer)
 * @retval None
 */
void removeCacheData(LRUCacheManager *cacheManager, CacheData *cacheData);

/**
 * @brief  Returns if queue is full or not 
 * @param  *lruCacheManager: Cache manager address (pointer)
 * @retval Returns if queue is full or not 
 */
bool isCacheFull(LRUCacheManager *lruCacheManager);

/**
 * @brief  Invalidates all cache entries
 * @param  *lruCacheManager: Cache manager address (pointer)
 * @retval None
 */
void invalidateCache(LRUCacheManager *lruCacheManager);

/**
 * @brief  Destructor
 * @param  *lruCacheManager: LRU Cache manager pointer
 * @retval None
 */
void LRUCacheManager_Free(LRUCacheManager *lruCacheManager);

#endif