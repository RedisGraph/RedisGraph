/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "cache_array.h"
#include "../../../deps/rax/rax.h"

/**
 * @brief Key-value cache with hard limit of items stored. Uses LRU policy for cache eviction.
 * Stores actual copy (memcpy) of the object itself.
 */
typedef struct Cache {
	uint cap;                          // Cache capacity.
	uint size;                         // Cache current size.
	long long counter;                 // Atomic counter of read operations to the cache.
	rax *lookup;                       // Rax of queries to cache values for fast lookups.
	CacheEntry *arr;                   // Array of cache elements.
	CacheEntryFreeFunc free_item;      // Function that frees cache entry value.
	CacheEntryCopyFunc copy_item;      // Function that copies cache entry value.
	pthread_rwlock_t _cache_rwlock;    // Read-write lock to protect access to the cache.
} Cache;

/**
 * @brief  Initialize a cache.
 * @param  size: Number of entries.
 * @param  freeCB: callback for freeing the stored values.
 *                 Note: if the original object is a nested compound object,
 *                 supply an appropriate function to avoid double resource releasing
 * @retval cache pointer - Initialized empty cache.
 */
Cache *Cache_New(uint size, CacheEntryFreeFunc freeFunc, CacheEntryCopyFunc copyFunc);

/**
 * @brief  Returns a value if it is cached, NULL otherwise.
 * @param  *cache: cache pointer.
 * @param  *key: Key to look for.
 * @retval  pointer with the cached answer, NULL if the key isn't cached.
 */
void *Cache_GetValue(Cache *cache, const char *key);

/**
 * @brief  Stores value under key within the cache.
 * @note   In case the cache is full, this operation causes a cache eviction.
 * @param  *cache: cache pointer.
 * @param  *key: Key for associating with value.
 * @param  *value: pointer with the relevant value.
 * @retval A copy of the given value if a new item was added to the cache, the original value if it was already exist.
 */
void *Cache_SetGetValue(Cache *cache, const char *key, void *value);

/**
 * @brief  Destroy a cache and free all of the stored items.
 * @param  *cache: cache pointer
 */
void Cache_Free(Cache *cache);

