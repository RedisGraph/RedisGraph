#include "lru_cache_manager.h"
#include "../../util/rmalloc.h"


CacheData *evictFromCache(LRUCacheManager *lruCacheManager)
{
  return (CacheData *)dequeue(lruCacheManager->queue);
}

CacheData *addToCache(LRUCacheManager *lruCacheManager, const char *hashKey, ResultSet* resultSet)
{
  LRUNode *newNode = enqueue(lruCacheManager->queue, hashKey, resultSet);
  return (CacheData *)newNode;
}
void increaseImportance(LRUCacheManager *cacheManager, void *cacheData)
{
  return moveToHead(cacheManager->queue, (LRUNode *) cacheData);
}

bool isCacheFull(LRUCacheManager *lruCacheManager)
{
  return isFullQueue(lruCacheManager->queue);
}

void invalidateCache(LRUCacheManager *lruCacheManager){
  emptyQueue(lruCacheManager->queue);
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
