#include "./lru_node.h"
#include "../../util/rmalloc.h"

  LRUNode *initLRUNode(LRUNode *node, const char *hashKey){
  node->next = NULL;
  node->prev = NULL;
  CacheData *cacheData = (CacheData *)node;
  memcpy(cacheData->hashKey, hashKey, HASH_KEY_LENGTH);
  return node;
}





