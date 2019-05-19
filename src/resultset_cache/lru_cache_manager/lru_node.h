#ifndef LRU_NODE_H
#define LRU_NODE_H
#include"../result_set_cache_includes.h"
#include "../cache_data.h"

typedef struct LRUNode LRUNode;

struct LRUNode {
  struct CacheData cacheData;
  LRUNode *prev;
  LRUNode *next;
};

LRUNode *initLRUNode(LRUNode* node, const char *hashKey, ResultSet* resultSet);

#endif