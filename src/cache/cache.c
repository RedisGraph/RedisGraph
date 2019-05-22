/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "cache.h"

Cache *Cache_New(size_t cacheSize, cacheValueFreeFunc freeCB)
{
    // memory allocations
    Cache *cache = rm_malloc(sizeof(Cache));
    // members initialization
    cache->lruCacheManager = LRUCacheManager_New(cacheSize, freeCB);
    cache->raxCacheStorage = RaxCacheStorage_New();
    cache->isValid = true;
    pthread_rwlock_init(&cache->rwlock, NULL);
    return cache;
}

void Cache_Free(Cache *cache)
{
    // members destructors
    LRUCacheManager_Free(cache->lruCacheManager);
    RaxCacheStorage_Free(cache->raxCacheStorage);
    pthread_rwlock_destroy(&cache->rwlock);
    // memory release
    rm_free(cache);
}

void *getCacheValue(Cache *cache, const char *query, size_t queryLength)
{
    // hash query
    unsigned long long const hashKey = hashQuery(query, queryLength);
    CacheData *cacheData = NULL;
    // acquire read lock
    pthread_rwlock_rdlock(&cache->rwlock);

    if (cache->isValid)
    {
        // get result set
        cacheData = getFromCache(cache->raxCacheStorage, (unsigned char *)&hashKey);
        if (cacheData)
        {
            increaseImportance(cache->lruCacheManager, cacheData);
        }
    }

    // free lock
    pthread_rwlock_unlock(&cache->rwlock);
    return cacheData ? cacheData->cacheValue : NULL;
}

void storeCacheValue(Cache *cache, const char *query, size_t queryLength, void *value)
{
    // hash query
    unsigned long long const hashKey = hashQuery(query, queryLength);
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    //if cache is in valid state
    if (cache->isValid)
    {
        // remove less recently used query
        if (isCacheFull(cache->lruCacheManager))
        {
            // get cache data from cache manager
            CacheData *evictedCacheData = evictFromCache(cache->lruCacheManager);
            // remove from storage
            removeFromCache(cache->raxCacheStorage, evictedCacheData);
        }
        // add to cache manager
        CacheData *insertedCacheData = addToCache(cache->lruCacheManager, hashKey, value);
        // store in storage
        insertToCache(cache->raxCacheStorage, insertedCacheData);
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

void removeCacheValue(Cache *cache, const char *query, size_t queryLength)
{
    // hash query
    unsigned long long const hashKey = hashQuery(query, queryLength);
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    // get result set
    CacheData *cacheData = getFromCache(cache->raxCacheStorage, (unsigned char *)&hashKey);
    if (cacheData)
    {
        removeCacheData(cache->lruCacheManager, cacheData);
        removeFromCache(cache->raxCacheStorage, cacheData);
    }
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void clearCache(Cache *cache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    // invalidate cache manager
    invalidateCache(cache->lruCacheManager);
    // clear storage
    clearCacheStorage(cache->raxCacheStorage);
    cache->isValid = true;
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}