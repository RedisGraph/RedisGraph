/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef RAX_CACHE_STORAGE_H
#define RAX_CACHE_STORAGE_H
#include "../cache_includes.h"
#include "../cache_data.h"
#include "rax/rax.h"

/**
 * @brief  Struct to store cache entries pointers according to their keys
 *         Holds a RAX radix tree from efficent key finding - basicaly a wrapper around RAX
 *         Each key is a charecter array in size of HASH_KEY_LENGTH
 *         Each value is a pointer to a cache entry stored in a cache manager
 * @retval None
 */
typedef struct RaxCacheStorage
{
    rax *rt;        // Rax radix tree
} RaxCacheStorage;

/**
 * @brief  Initlizes a new RAX storage  
 * @retval pointer to intilized RAX storage
 */
RaxCacheStorage *RaxCacheStorage_New();

/**
 * @brief  Inserts a new cache entry address to storage 
 * @param  *raxCacheStorage: Rax storage (pointer)
 * @param  *cacheData: entry value
 * @retval None
 */
void insertToCache(RaxCacheStorage *raxCacheStorage, CacheData *cacheData);

/**
 * @brief  Removes an key (and its entry) from RAX 
 * @param  *raxCacheStorage: Rax storage (pointer)
 * @param  *cacheData : cacheData which its key is removed from rax
 * @retval None
 */
void removeFromCache(RaxCacheStorage *raxCacheStorage, CacheData *cacheData);

/**
 * @brief  Returns a result set according to its stored key
 * @param  *raxCacheStorage: Rax storage (pointer)
 * @param  *hashKey: entry key - a charecter array in size of HASH_KEY_LENGTH 
 * @retval a cache entry pointer, stored in the cache entry with the specific key
 */
CacheData *getFromCache(RaxCacheStorage *raxCacheStorage, unsigned char *hashKey);

/**
 * @brief  clears the cache storage
 * @param  *raxCacheStorage: Rax storage (pointer)
 * @retval None
 */
void clearCacheStorage(RaxCacheStorage *raxCacheStorage);

/**
 * @brief  Destructor 
 * @param  *raxCacheStorage: Rax storage (pointer)
 * @retval None
 */
void RaxCacheStorage_Free(RaxCacheStorage *raxCacheStorage);

#endif