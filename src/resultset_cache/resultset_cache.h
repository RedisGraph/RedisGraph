/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <pthread.h>
#include "../cache/cache.h"
#include "../resultset/resultset.h"


#define RESULTSET_CACHE_ENTRY_NUMBER 50

/**
 * @brief  Returns a cache for a graph by its name
 * @note   
 * @param  *graphName: graph name (charecters array)
 * @param  graphNameLength: 
 * @retval cache pointer
 */
Cache *getGraphCache(const char *graphName, size_t graphNameLength);

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