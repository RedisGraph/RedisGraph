#ifndef LRU_CACHE_MANAGER_H
#define LRU_CACHE_MANAGER_H
#include "../cache_manager.h"
#include "./lru_queue.h"

typedef struct LRUCacheManager LRUCacheManager;

struct LRUCacheManager {
  struct CacheManager cacheManager;
  LRUQueue* queue;
};

CacheManager* LRUCacheManager_New(size_t);
CacheManager *LRUCacheManger_Init(LRUCacheManager *lruCacheManager);

#endif