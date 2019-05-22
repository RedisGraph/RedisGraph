/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./lru_node.h"

LRUNode *initLRUNode(LRUNode *node, unsigned long long const hashKey, ResultSet *resultSet)
{
  // new node - no next and prev
  node->next = NULL;
  node->prev = NULL;
  // copy key
  node->cacheData.hashKey = hashKey;
  
  // if node was in use before, release the result set
  if (node->cacheData.isDirty)
  {
    ResultSet_Free(node->cacheData.cacheValue);
  }
  // set new value and mark as used
  node->cacheData.cacheValue = resultSet;
  node->cacheData.isDirty = true;

  return node;
}
