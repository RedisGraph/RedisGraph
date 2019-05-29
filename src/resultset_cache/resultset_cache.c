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

Cache *addGraphCache(const char *graphName, size_t graphNameLength)
{
    // if not exists, create new and return
    // acquire write lock
    pthread_rwlock_wrlock(&registeredCaches_rwlock);
    raxInsert(registeredCaches, (unsigned char *)graphName, graphNameLength, cacheNew(RESULTSET_CACHE_ENTRY_NUMBER, (cacheValueFreeFunc)ResultSet_Free), NULL);
    // free lock
    pthread_rwlock_unlock(&registeredCaches_rwlock);
    return getGraphCache(graphName, graphNameLength);
}

Cache * getGraphCache(const char *graphName, size_t graphNameLength)
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
    Cache *resultSetCache = raxFind(registeredCaches, (unsigned char *)graphName, graphNameLength);
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
    Cache *resultSetCache = raxFind(registeredCaches, (unsigned char *)graphName, graphNameLength);
    cacheFree(resultSetCache);
    raxRemove(registeredCaches, (unsigned char *)graphName, graphNameLength, NULL);
    // free lock
    pthread_rwlock_unlock(&registeredCaches_rwlock);
}

ResultSet *graphCacheGet(const char *graphName, const char *query)
{
    Cache *graphCache = getGraphCache(graphName, strlen(graphName));
    return cacheGetValue(graphCache, query, strlen(query));
}

void graphCacheSet(const char *graphName, const char *query, ResultSet *resultset)
{
    Cache *graphCache = getGraphCache(graphName, strlen(graphName));
    cacheSetValue(graphCache, query, strlen(query), resultset);
}

void graphCacheRemove(const char *graphName, const char *query)
{
    Cache *graphCache = getGraphCache(graphName, strlen(graphName));
    cacheRemoveValue(graphCache, query, strlen(query));
}

void graphCacheMarkInvalid(const char *graphName)
{
    Cache *graphCache = getGraphCache(graphName, strlen(graphName));
    cacheMarkInvalid(graphCache);
}

void graphCacheInvalidate(const char *graphName)
{
    Cache *graphCahe = getGraphCache(graphName, strlen(graphName));
    cacheClear(graphCahe);
}
