#include "lru_cache_manager.h"
#include "../../util/rmalloc.h"


CacheData *evictFromCache(LRUCacheManager *lruCacheManager)
{
  return (CacheData *)dequeue(lruCacheManager->queue);
}

CacheData *addToCache(LRUCacheManager *lruCacheManager, const char *hashKey)
{
  LRUNode *newNode = enqueue(hashKey, lruCacheManager->queue);
  return (CacheData *)newNode;
}
void increaseImportance(LRUCacheManager *cacheManager, void *cacheData)
{
  return moveToHead((LRUNode *)cacheData, cacheManager->queue);
}

bool isCacheFull(LRUCacheManager *lruCacheManager)
{
  return isFullQueue(lruCacheManager->queue);
}

// create, init & delete
void LRUCacheManager_Free(LRUCacheManager *lruCacheManager)
{
  LRUQueue_Free(lruCacheManager->queue);
  rm_free(lruCacheManager);
}

LRUCacheManager *LRUCacheManager_New(size_t size)
{
  LRUCacheManager *lruCacheManager = rm_malloc(sizeof(LRUCacheManager));
  lruCacheManager->queue = LRUQueue_New(size);
  return lruCacheManager;
}
