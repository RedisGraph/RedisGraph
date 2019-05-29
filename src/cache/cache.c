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

void cacheDataFree(void* voidPtr){
    CacheData *cacheData = voidPtr;
    cacheData->freeFunc(cacheData->value);
}

Cache *cacheNew(size_t cacheSize, cacheValueFreeFunc freeCB)
{
    // memory allocations
    Cache *cache = rm_malloc(sizeof(Cache));
    // members initialization
    cache->lruQueue = lruQueueNew(cacheSize, sizeof(CacheData), (lruDataFreeFunc)cacheDataFree);
    cache->rt = raxNew();
    cache->isValid = true;
    cache->cacheValueFree = freeCB;
    pthread_rwlock_init(&cache->rwlock, NULL);
    return cache;
}

void *cacheGetValue(Cache *cache, const char *key, size_t keyLen)
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
            lruQueueMoveToHead(cache->lruQueue, cacheData);
        }
    }

    // free lock
    pthread_rwlock_unlock(&cache->rwlock);
    return (cacheData && cacheData != raxNotFound) ? cacheData->value : NULL;
}

void cacheSetValue(Cache *cache, const char *key, size_t keyLen, void *value)
{
    unsigned long long const hashKey = hashQuery(key, keyLen);
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    //if cache is in valid state
    if (cache->isValid)
    {
        // remove less recently used query
        if (lruQueueIsFull(cache->lruQueue))
        {
            // get cache data from queue
            CacheData *evictedCacheData = (CacheData *)lruQueueDequeue(cache->lruQueue);
            // remove from storage
            raxRemove(cache->rt, (unsigned char *)&evictedCacheData->hashKey, HASH_KEY_LENGTH, NULL);
        }
        // add to lruQueue
        CacheData cacheData;
        cacheData.hashKey = hashKey;
        cacheData.value = value;
        cacheData.freeFunc = cache->cacheValueFree;
        CacheData *insertedCacheData = (CacheData *)lruQueueEnqueue(cache->lruQueue, &cacheData);
        // store in storage
        raxInsert(cache->rt, (unsigned char *)&insertedCacheData->hashKey, HASH_KEY_LENGTH, insertedCacheData, NULL);
    }
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void cacheRemoveValue(Cache *cache, const char *key, size_t keyLen)
{
    unsigned long long const hashKey = hashQuery(key, keyLen);
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    // get result set
    CacheData *cacheData = raxFind(cache->rt, (unsigned char*)&hashKey, HASH_KEY_LENGTH);
    if (cacheData != raxNotFound)
    {
        lruQueueRemoveFromQueue(cache->lruQueue, cacheData);
        raxRemove(cache->rt, (unsigned char *)&cacheData->hashKey, HASH_KEY_LENGTH, NULL);
    }
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void cacheClear(Cache *cache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    // empty the lru queue
    lruQueueEmptyQueue(cache->lruQueue);
    // clear storage
   // free rax
    raxFree(cache->rt);
    // re alloc rax
    cache->rt = raxNew();
    cache->isValid = true;
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void cacheMarkInvalid(Cache *cache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&cache->rwlock);
    cache->isValid = false;
    // unlock
    pthread_rwlock_unlock(&cache->rwlock);
}

void cacheFree(Cache *cache)
{
    // members destructors
    lruQueueFree(cache->lruQueue);
    raxFree(cache->rt);
    pthread_rwlock_destroy(&cache->rwlock);
    // memory release
    rm_free(cache);
}

