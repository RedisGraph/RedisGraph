/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef LRU_NODE_H
#define LRU_NODE_H
#include"../result_set_cache_includes.h"
#include "../cache_data.h"

// pre declaration 
typedef struct LRUNode LRUNode;

/**
 * @brief  A struct that wraps cache data, for LRUQueue usage
 */
struct LRUNode {
  struct CacheData cacheData;     // CacheData
  LRUNode *prev;                  // Next Node in the queue
  LRUNode *next;                  // Previous node in the queue
};

/**
 * @brief  Initialize a LRU node, and its undelying CacheData
 * @note   
 * @param  node: Node's address (pointer)
 * @param  hashKey: Node's cache key (for CacheData)
 * @param  resultSet: Node's result value (for CacheData)
 * @retval Initilized LRU node (pointer)
 */
LRUNode *initLRUNode(LRUNode* node, const char *hashKey, ResultSet* resultSet);

#endif