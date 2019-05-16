#include "./lru_node.h"
#include "../../util/rmalloc.h"

void LRUNodeCacheData_Free(CacheData *cacheData) {
  LRUNode_Free((LRUNode *)cacheData);
}

  LRUNode *initLRUNode(LRUNode *node, const char *hashKey){
  node->next = NULL;
  node->prev = NULL;
  CacheData *cacheData = (CacheData *)node;
  memcpy(cacheData->hashKey, hashKey, HASH_KEY_LENGTH);
  cacheData->cacheData_Free = &LRUNodeCacheData_Free;
  return node;
}

LRUNode *LRUNode_New(const char *hashKey) {
  LRUNode *node = rm_malloc(sizeof(LRUNode));
  return initLRUNode(node, hashKey);
}

void LRUNode_Free(LRUNode *node) {
  rm_free(node);
}

