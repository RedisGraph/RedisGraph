/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset_cache.h"

ResultSetCache *ResultSetCache_New(size_t cacheSize){
    // memory allocations
    ResultSetCache *resultSetCache = rm_malloc(sizeof(ResultSetCache));
    // members initialization
    resultSetCache->lruCacheManager = LRUCacheManager_New(cacheSize);
    resultSetCache->raxCacheStorage = RaxCacheStorage_New();
    pthread_rwlock_init(&resultSetCache->rwlock, NULL);
    return resultSetCache;
}


void ResultSetCache_Free(ResultSetCache *resultSetCache){
    // members destructors
    LRUCacheManager_Free(resultSetCache->lruCacheManager);
    RaxCacheStorage_Free(resultSetCache->raxCacheStorage);
    pthread_rwlock_destroy(&resultSetCache->rwlock);
    // memory release
    rm_free(resultSetCache);
}

ResultSet *getResultSet(ResultSetCache *resultSetCache, const char *query)
{
    // hash query
    char hashKey[HASH_KEY_LENGTH];
    hashQuery(query, HASH_KEY_LENGTH, hashKey);
    // acquire read lock
    pthread_rwlock_rdlock(&resultSetCache->rwlock);
    // get result set
    CacheData* cacheData = getFromCache(resultSetCache->raxCacheStorage, hashKey);
    if (cacheData){
        increaseImportance(resultSetCache->lruCacheManager, cacheData);
    }
   
    // free lock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
    return cacheData ? cacheData->resultSet : NULL;
}

void storeResultSet(ResultSetCache *resultSetCache, const char *query, ResultSet *resultSet)
{
    // hash query
    char hashKey[HASH_KEY_LENGTH];
    hashQuery(query, HASH_KEY_LENGTH, hashKey);
    // acquire write lock
    pthread_rwlock_wrlock(&resultSetCache->rwlock);
    // remove less recently used query
    if (isCacheFull(resultSetCache->lruCacheManager)){
        // get cache data from cache manager
        CacheData *evictedCacheData = evictFromCache(resultSetCache->lruCacheManager);
        // remove from storage
        removeFromCache(resultSetCache->raxCacheStorage, evictedCacheData->hashKey); 
    }
    // add to cache manager
    CacheData *insertedCacheData = addToCache(resultSetCache->lruCacheManager, hashKey, resultSet);
    // store in storage
    insertToCache(resultSetCache->raxCacheStorage,  insertedCacheData);
    // unlock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
}

void clearCache(ResultSetCache *resultSetCache){
    // acquire read lock
    pthread_rwlock_wrlock(&resultSetCache->rwlock);
    // invalidate cache manager
    invalidateCache(resultSetCache->lruCacheManager);
    // clear storage
    clearCacheStorage(resultSetCache->raxCacheStorage);
    // unlock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
}