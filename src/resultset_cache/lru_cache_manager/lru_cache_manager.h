#ifndef LRU_CACHE_MANAGER_H
#define LRU_CACHE_MANAGER_H
#include "../result_set_cache_includes.h"
#include "./lru_queue.h"

typedef struct LRUCacheManager LRUCacheManager;

struct LRUCacheManager {
  LRUQueue* queue;
};

LRUCacheManager *LRUCacheManager_New(size_t);
CacheData *addToCache(LRUCacheManager *lruCacheManager, const char *hashKey, ResultSet* resultSet);
CacheData *evictFromCache(LRUCacheManager *lruCacheManager);
void increaseImportance(LRUCacheManager *cacheManager, void *cacheData);
bool isCacheFull(LRUCacheManager *lruCacheManager);
void invalidateCache(LRUCacheManager *lruCacheManager);
void LRUCacheManager_Free(LRUCacheManager *lruCacheManager);

#endif