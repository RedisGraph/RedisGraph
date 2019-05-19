#ifndef LRU_NODE_H
#define LRU_NODE_H
#include "../cache_data.h"

typedef struct LRUNode LRUNode;

struct LRUNode {
  struct CacheData cacheData;
  LRUNode *prev;
  LRUNode *next;
};

LRUNode *initLRUNode(LRUNode* node, const char *hashKey);

#endif