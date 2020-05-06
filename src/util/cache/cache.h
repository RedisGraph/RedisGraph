/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../priority_queue.h"
#include "rax.h"

#define HASH_KEY_LENGTH 8

typedef uint64_t hash_key_t;
typedef void (*cacheValueFreeFunc)(void *);

/**
 * @brief  Struct for holding cache data.
 */
typedef struct CacheData {
	hash_key_t hashKey;             // CacheData key - 64 bit hashed key.
	void *value;                    // Value to be stored in cache.
	cacheValueFreeFunc freeFunc;    // function to free the stored value.
} CacheData;


/**
 * @brief Key-value cache with hard limit of items stored. Uses LRU policy for cache eviction.
 * Stores actual copy (memcpy) of the object itself.
 */
typedef struct Cache {
	rax *lookup;                        // Storage - Rax radix tree.
	PriorityQueue *priorityQueue;       // Priority Queue for maintaining eviction policy.
	cacheValueFreeFunc cacheValueFree;  // function pointer to free cache value.
} Cache;

/**
 * @brief  Initialize a cache.
 * @param  cacheSize: Number of entries.
 * @param  freeCB: callback for freeing the stored values.
 *                 Note: if the original object is a nested compound object,
 *                 supply an appropriate function to avoid double resource releasing
 * @retval cache pointer - Initialized empty cache.
 */
Cache *Cache_New(uint cacheSize, cacheValueFreeFunc freeCB);

/**
 * @brief  Returns a value if it is cached, NULL otherwise.
 * @param  *cache: cache pointer.
 * @param  *key: Key to look for (bytes array).
 * @retval  pointer with the cached answer, NULL if the key isn't cached.
 */
void *Cache_GetValue(Cache *cache, const char *key);

/**
 * @brief  Stores value under key within the cache.
 * @param  *cache: cache pointer.
 * @param  *key: Key for associating with value (bytes array).
 * @param  *value: pointer with the relevant value.
 */
void Cache_SetValue(Cache *cache, const char *key, void *value);

/**
 * @brief  Destroy a cache
 * @param  *cache: cache pointer
 */
void Cache_Free(Cache *cache);

