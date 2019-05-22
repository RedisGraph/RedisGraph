/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "resultset_cache.h"
/**
 * @brief  global rax to store in use caches
 * @note   
 * @retval None
 */
static rax *registeredCaches = NULL;
static pthread_rwlock_t registeredCaches_rwlock; // Read-Write lock for the global rax

ResultSetCache *addGraphCache(const char *graphName, size_t graphNameLength)
{
    // if not exists, create new and return
    // acquire write lock
    pthread_rwlock_wrlock(&registeredCaches_rwlock);
    raxInsert(registeredCaches, (unsigned char *)graphName, graphNameLength, ResultSetCache_New(RESULTSET_CACHE_ENTRY_NUMBER), NULL);
    // free lock
    pthread_rwlock_unlock(&registeredCaches_rwlock);
    return getGraphCache(graphName, graphNameLength);
}


ResultSetCache *
getGraphCache(const char *graphName, size_t graphNameLength)
{
    // not initialized graphs container
    if (registeredCaches == NULL)
    {
        // initialize graphs container
        // no need to sync will always called first from main thread only!
        registeredCaches = raxNew();
        pthread_rwlock_init(&registeredCaches_rwlock, NULL);
    }

    // find cache
    pthread_rwlock_rdlock(&registeredCaches_rwlock);
    ResultSetCache *resultSetCache = raxFind(registeredCaches, (unsigned char *)graphName, graphNameLength);
    // free lock
    pthread_rwlock_unlock(&registeredCaches_rwlock);
    if (resultSetCache == raxNotFound)
    {
        //create new cache
        return addGraphCache(graphName, graphNameLength);
    }
    return resultSetCache;
}

void removeGraphCache(const char *graphName)
{
    size_t graphNameLength = strlen(graphName);
    // if not exists, create new and return
    // acquire write lock
    pthread_rwlock_wrlock(&registeredCaches_rwlock);
    ResultSetCache *resultSetCache = raxFind(registeredCaches, (unsigned char *)graphName, graphNameLength);
    ResultSetCache_Free(resultSetCache);
    raxRemove(registeredCaches, (unsigned char *)graphName, graphNameLength, NULL);
    // free lock
    pthread_rwlock_unlock(&registeredCaches_rwlock);
}

ResultSet *getGraphCacheResultSet(const char *graphName, const char *query)
{
    ResultSetCache *graphCache = getGraphCache(graphName, strlen(graphName));
    return getResultSet(graphCache, query, strlen(query));
}

void setGraphCacheResultSet(const char *graphName, const char *query, ResultSet *resultset)
{
    ResultSetCache *graphCache = getGraphCache(graphName, strlen(graphName));
    storeResultSet(graphCache, query, strlen(query), resultset);
}

ResultSetCache *ResultSetCache_New(size_t cacheSize)
{
    // memory allocations
    ResultSetCache *resultSetCache = rm_malloc(sizeof(ResultSetCache));
    // members initialization
    resultSetCache->lruCacheManager = LRUCacheManager_New(cacheSize);
    resultSetCache->raxCacheStorage = RaxCacheStorage_New();
    resultSetCache->isValid = true;
    pthread_rwlock_init(&resultSetCache->rwlock, NULL);
    return resultSetCache;
}

void ResultSetCache_Free(ResultSetCache *resultSetCache)
{
    // members destructors
    LRUCacheManager_Free(resultSetCache->lruCacheManager);
    RaxCacheStorage_Free(resultSetCache->raxCacheStorage);
    pthread_rwlock_destroy(&resultSetCache->rwlock);
    // memory release
    rm_free(resultSetCache);
}

ResultSet *getResultSet(ResultSetCache *resultSetCache, const char *query, size_t queryLength)
{
    // hash query
    unsigned long long const hashKey = hashQuery(query, queryLength);
    CacheData *cacheData = NULL;
    // acquire read lock
    pthread_rwlock_rdlock(&resultSetCache->rwlock);
   
    if (resultSetCache->isValid)
    {
        // get result set
        cacheData = getFromCache(resultSetCache->raxCacheStorage, (unsigned char*)&hashKey);
        if (cacheData)
        {
            increaseImportance(resultSetCache->lruCacheManager, cacheData);
        }
    }

    // free lock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
    return cacheData ? cacheData->resultSet : NULL;
}

void storeResultSet(ResultSetCache *resultSetCache, const char *query, size_t queryLength, ResultSet *resultSet)
{
    // hash query
    unsigned long long const hashKey = hashQuery(query, queryLength);
    // acquire write lock
    pthread_rwlock_wrlock(&resultSetCache->rwlock);
    //if cache is in valid state
    if (resultSetCache->isValid)
    {
        // remove less recently used query
        if (isCacheFull(resultSetCache->lruCacheManager))
        {
            // get cache data from cache manager
            CacheData *evictedCacheData = evictFromCache(resultSetCache->lruCacheManager);
            // remove from storage
            removeFromCache(resultSetCache->raxCacheStorage, evictedCacheData);
        }
        // add to cache manager
        CacheData *insertedCacheData = addToCache(resultSetCache->lruCacheManager, hashKey, resultSet);
        // store in storage
        insertToCache(resultSetCache->raxCacheStorage, insertedCacheData);
    }
    // unlock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
}

void markCacheInvalid(ResultSetCache *resultSetCache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&resultSetCache->rwlock);
    resultSetCache->isValid = false;
    // unlock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
}

void markGraphCacheInvalid(const char *graphName)
{
    ResultSetCache *graphCache = getGraphCache(graphName, strlen(graphName));
    markCacheInvalid(graphCache);
}
void clearCache(ResultSetCache *resultSetCache)
{
    // acquire write lock
    pthread_rwlock_wrlock(&resultSetCache->rwlock);
    // invalidate cache manager
    invalidateCache(resultSetCache->lruCacheManager);
    // clear storage
    clearCacheStorage(resultSetCache->raxCacheStorage);
    resultSetCache->isValid = true;
    // unlock
    pthread_rwlock_unlock(&resultSetCache->rwlock);
}

void invalidateGraphCache(const char *graphName)
{
    ResultSetCache *graphCahe = getGraphCache(graphName, strlen(graphName));
    clearCache(graphCahe);
}
