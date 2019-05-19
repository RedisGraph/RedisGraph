#include "./lru_node.h"


  LRUNode *initLRUNode(LRUNode *node, const char *hashKey, ResultSet* resultSet){
  node->next = NULL;
  node->prev = NULL;
  memcpy(&node->cacheData.hashKey, hashKey, HASH_KEY_LENGTH);
  if (node->cacheData.isDirty){
    ResultSet_Free(node->cacheData.resultSet);
  }
  node->cacheData.resultSet = resultSet;
  node->cacheData.isDirty = true;

  return node;
}





