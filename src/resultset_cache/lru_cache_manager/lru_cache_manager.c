#include "lru_cache_manager.h"
#include "../../util/rmalloc.h"
#include <stdbool.h>
#include "./lru_queue.h"
#include <stdio.h>
#include <string.h>

// inner implementation

bool _isCacheFull(LRUCacheManager *lruCacheManager)
{
  return isFullQueue(lruCacheManager->queue);
}

CacheData* _evictFromCache(LRUCacheManager *lruCacheManager){
  return (CacheData *)dequeue(lruCacheManager->queue);
}

LRUNode *_addToCache(const char *hashKey, LRUCacheManager *cacheManager)
{
  LRUNode *newNode = enqueue(hashKey, cacheManager->queue);
  return newNode;
}
void _increaseImportance(LRUNode *lruNode, LRUCacheManager *cacheManager)
{
  moveToHead(lruNode, cacheManager->queue);
}

// functions wrappers

CacheData *evictFromCache(CacheManager *cacheManager)
{
  return _evictFromCache((LRUCacheManager *)cacheManager);
}

CacheData *addToCache(const char *hashKey, CacheManager *cacheManager)
{
  return (CacheData *)_addToCache(hashKey, (LRUCacheManager *)cacheManager);
}
void increaseImportance(void *cacheData, CacheManager *cacheManager)
{
  return _increaseImportance((LRUNode *)cacheData, (LRUCacheManager *)cacheManager);
}

bool isCacheFull(CacheManager *cacheManager)
{
  return _isCacheFull((LRUCacheManager *)cacheManager);
}

// create, init & delete
void LRUCacheManager_Free(CacheManager *cacheManager)
{
  LRUCacheManager *lRUCacheManager = (LRUCacheManager *)cacheManager;
  LRUQueue_Free(lRUCacheManager->queue);
  rm_free(lRUCacheManager);
}

CacheManager *InitLRUCacheManager(LRUCacheManager *lruCacheManager)
{
  CacheManager *cacheManager = (CacheManager *)lruCacheManager;
 
  cacheManager->addToCache = &addToCache;
  cacheManager->increaseImportance = &increaseImportance;
  cacheManager->cacheManager_free = &LRUCacheManager_Free;
  cacheManager->isCacheFull = &isCacheFull;
  cacheManager->evictFromCache = &evictFromCache;

  return cacheManager;
}

CacheManager *LRUCacheManager_New(size_t size)
{
  LRUCacheManager *lruCacheManager = rm_malloc(sizeof(LRUCacheManager));
  lruCacheManager->queue = LRUQueue_New(size);
  return InitLRUCacheManager(lruCacheManager);
}
