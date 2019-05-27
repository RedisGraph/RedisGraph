/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cache.h"
#include "xxhash/xxhash.h"

/**
 * @brief  Hash a query using XXHASH into a 64 bit
 * @param  *key - string to be hashed
 * @param  keyLen: key length
 * @retval hash value
 */
hash_key_t hashQuery(const char *key, size_t keyLen)
{
    return  XXH64(key, keyLen, 0);
}

Cache *Cache_New(size_t cacheSize, cacheValueFreeFunc freeCB)
{
    // memory allocations
    Cache *cache = rm_malloc(sizeof(Cache));
    // members initialization
    cache->lruQueue = LRUQueue_New(cacheSize, freeCB);
    cache->rt = raxNew();
    cache->isValid = true;
    pthread_rwlock_init(&cache->rwlock, NULL);
    return cache;
}

void Cache_Free(Cache *cache)
{
    // members destructors
    LRUQueue_Free(cache->lruQueue);
    raxFree(cache->rt);
    pthread_rwlock_destroy(&cache->rwlock);
    // memory release
    rm_free(cache);
}

void *getCacheValue(Cache *cache, const char *key, size_t keyLen)
{
    unsigned long long const hashKey = hashQuery(key, keyLen);
    CacheData *cacheData = NULL;
    // acquire read lock
    pthread_rwlock_rdlock(&cache->rwlock);

    if (cache->isValid)
    {
        cacheData = raxFind(cache->rt, (unsigned char*)&hashKey, HASH_KEY_LENGTH);
        if (cacheData != raxNotFound)
        {
            moveToHead(cache->lruQueue, (LRUNode *) cacheData);
        }
    }

    // free lock
    pthread_rwlock_unlock(&cache->rwlock);
    return (cacheData && cacheData != raxNotFound) ? cacheData->cacheValue : NULL;
}

void storeCacheValue(Cache *cache, const char *key, size_t keyLen, void *value)
{
    unsigned long long const hashKey = hashQuery(key, keyLen);
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    //if cache is in valid state
    if (cache->isValid)
    {
        // remove less recently used query
        if (isFullQueue(cache->lruQueue))
        {
            // get cache data from queue
            CacheData *evictedCacheData = (CacheData *)dequeue(cache->lruQueue);
            // remove from storage
            raxRemove(cache->rt, (unsigned char *)&evictedCacheData->hashKey, HASH_KEY_LENGTH, NULL);
        }
        // add to lruQueue
        CacheData *insertedCacheData = (CacheData *) enqueue(cache->lruQueue, hashKey, value);
        // store in storage
        raxInsert(cache->rt, (unsigned char *)&insertedCacheData->hashKey, HASH_KEY_LENGTH, insertedCacheData, NULL);
    }
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void markCacheInvalid(Cache *cache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    cache->isValid = false;
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void removeCacheValue(Cache *cache, const char *key, size_t keyLen)
{
    unsigned long long const hashKey = hashQuery(key, keyLen);
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    // get result set
    CacheData *cacheData = raxFind(cache->rt, (unsigned char*)&hashKey, HASH_KEY_LENGTH);
    if (cacheData != raxNotFound)
    {
        removeFromQueue(cache->lruQueue, (LRUNode *)cacheData);
        raxRemove(cache->rt, (unsigned char *)&cacheData->hashKey, HASH_KEY_LENGTH, NULL);
    }
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void clearCache(Cache *cache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    // empty the lru queue
    emptyQueue(cache->lruQueue);
    // clear storage
   // free rax
    raxFree(cache->rt);
    // re alloc rax
    cache->rt = raxNew();
    cache->isValid = true;
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}