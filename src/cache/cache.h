/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "./cache_includes.h"
#include "../util/lru_queue.h"
#include "rax/rax.h"
#include "pthread.h"

/**
 * Key-value cache with hard limit of items stored. Uses LRU policy for cache eviction.
 * Stores actual copy (memcpy) of the object itself
 */
typedef struct Cache {
    rax *rt;                            // Storage - Rax radix tree
    bool isValid;                       // Flag infoming read operations that write is about to happen
    LRUQueue *lruQueue;                 // LRU Queue for maintaining eviction policy
    cacheValueFreeFunc cacheValueFree;  // function pointer to free cache value
    pthread_rwlock_t rwlock; // Read-Write lock
} Cache;

/**
 * @brief  Initilize a  cache
 * @param  cacheSize: Number of entries
 * @param  freeCB: callback for freeing the stored values. 
 *                 Note: if the original object is a nested compound object, 
 *                       supply an appropriate function to avoid double resource releasing
 * @retval cache pointer - Initialized empty cache
 */
Cache *cacheNew(size_t cacheSize, cacheValueFreeFunc freeCB);

/**
 * @brief  Returns a value if it is cached, NULL otherwise  
 * @param  *cache: cache pointer
 * @param  *key: Key to look for (charecters array)
 * @param  keyLen: key length
 * @retval  pointer with the cached answer, NULL if the key isn't cached
 */
void *cacheGetValue(Cache *cache, const char *key, size_t keyLen);

/**
 * @brief  Sotres a key and value in the cache
 * @param  *cache: cache pointer
 * @param  *key: Key for associating with value (charecters array)
 * @param  keyLen: key length
 * @param  *value: pointer with the relevant value
 * @retval None
 */
void cacheSetValue(Cache *cache, const char *key, size_t keyLen, void *value);

/**
 * @brief  Removes a value from the cache according to its key
 * @param  *cache: cache pointer
 * @param  *key: Key to look for (charecters array)
 * @param  keyLen: key length
 * @retval None
 */
void cacheRemoveValue(Cache *cache, const char *key, size_t keyLen);

/**
 * @brief  Clears the cache from entries
 * @param  *cache: cache pointer
 * @retval None
 */
void cacheClear(Cache *cache);

/**
 * @brief  Marks cache is invalid for readers, by a writer which intend to write.
 * @note   
 * @param  *cache: 
 * @retval None
 */
void cacheMarkInvalid(Cache *cache);

/**
 * @brief  Destory a  cache
 * @param  *cache: cache pointer
 * @retval None
 */
void cacheFree(Cache *cache);
