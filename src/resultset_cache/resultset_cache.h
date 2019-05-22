/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef RESULTSET_CACHE_H
#define RESULTSET_CACHE_H
#include <pthread.h>
#include "result_set_cache_includes.h"
#include "./lru_cache_manager/lru_cache_manager.h"
#include "./rax_cache_storage/rax_cache_storage.h"
#include "./xxhash_query_hash/xxhash_query_hash.h"

//dvirdu
// number of entries
#define RESULTSET_CACHE_ENTRY_NUMBER 50

/**
 * A struct to for read queries cache storage. 
 * Holds a managment struct (LRU policy) and a RAX radix tree for storage.
 * A read-write lock is set for multi-threaded operatins.
 */
typedef struct ResultSetCache
{
    LRUCacheManager *lruCacheManager; // Cache Manager - LRU policy
    RaxCacheStorage *raxCacheStorage; // Storage - Rax radix tree
    pthread_rwlock_t rwlock;          // Read-Write lock
    bool isValid;
} ResultSetCache;

/**
 * @brief  Initilize a result-sets cache
 * @param  cacheSize: Cache size (in entries)
 * @retval ResultSetCache pointer - Initialized empty cache
 */
ResultSetCache *ResultSetCache_New(size_t cacheSize);

/**
 * @brief  Destory a results-sets cache
 * @param  *resultSetCache: ResultSetCache pointer
 * @retval None
 */
void ResultSetCache_Free(ResultSetCache *resultSetCache);

/**
 * @brief  Returns a ResultSet if it is cached, NULL otherwise  
 * @param  *resultSetCache: ResultSetCache pointer
 * @param  *query: Cypher query - charecters array
 * @retval ResultSet pointer with the cached answer, NULL if the query isn't cached
 */
ResultSet *getResultSet(ResultSetCache *resultSetCache, const char *query, size_t queryLength);

/**
 * @brief  Sotres a query in the cache
 * @param  *resultSetCache: ResultSetCache pointer
 * @param  *query: Cypher query to be associated with the result set- charecters array
 * @param  queryLength:
 * @param  *resultSet: ResultSet pointer with the relevant result
 * @retval None
 */
void storeResultSet(ResultSetCache *resultSetCache, const char *query, size_t queryLength, ResultSet *resultSet);

/**
 * @brief  Removes a result set from the cache
 * @param  *resultSetCache: ResultSetCache pointer
 * @param  *query: Cypher query to be associated with the result set- charecters array
 * @param  queryLength:
 * @param  *resultSet: ResultSet pointer with the relevant result
 * @retval None
 */
void removeResultSet(ResultSetCache *resultSetCache, const char *query, size_t queryLength);

/**
 * @brief  Clears the cache from entries
 * @param  *resultSetCache: ResultSetCache pointer
 * @retval None
 */
void clearCache(ResultSetCache *resultSetCache);

/**
 * @brief  Returns a cache for a graph by its name
 * @note   
 * @param  *graphName: graph name (charecters array)
 * @param  graphNameLength: 
 * @retval ResultSet cache pointer
 */
ResultSetCache *getGraphCache(const char *graphName, size_t graphNameLength);

/**
 * @brief  Get a query result set for a cache of a specific graph  
 * @param  *graphName: Graph name (key for the graph cache)
 * @param  *query:  query for getting the cached result set
 * @retval Relavant result set if the query was cached for this graph before. null otherwise
 */
ResultSet *graphCacheGet(const char *graphName, const char *query);

/**
 * @brief  set a query result set in a cache of a specific graph 
 * @param  *graphName:  Graph name (key for the graph cache)
 * @param  query: query for setting the result set, as a key
 * @param  *resultset: cache value
 * @retval None
 */
void graphCacheSet(const char *graphName, const char *query, ResultSet *resultset);

/**
 * @brief  removes a query result set for a cache of a specific graph  
 * @param  *graphName: Graph name (key for the graph cache)
 * @param  *query:  query for removing the cached result set
 * @retval None
 */
void graphCacheRemove(const char *graphName, const char *query);

/**
 * @brief Marks graph's cache as invalid, as part of graph modification preperations 
 * @param  *graphName: Graph's name (key to invalidate)
 * @retval None
 */
void graphCacheMarkInvalid(const char *graphName);

/**
 * @brief  The actual invalidation of a graph cache
 * @param  *graphName: Graph's name (key to invalidate)
 * @retval None
 */
void graphCacheInvalidate(const char *graphName);

/**
 * @brief  Delets a graph cache from the global rax which stores caches 
 * @param  *graphName: Graph's name (key to delete)
 * @retval None
 */
void removeGraphCache(const char *graphName);

#endif