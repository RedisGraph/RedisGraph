/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "lru_cache_manager.h"

LRUCacheManager *LRUCacheManager_New(size_t size, cacheValueFreeFunc freeCB)
{
  // memeory allocation
  LRUCacheManager *lruCacheManager = rm_malloc(sizeof(LRUCacheManager));
  // create new LRU queue
  lruCacheManager->queue = LRUQueue_New(size, freeCB);
  return lruCacheManager;
}

void LRUCacheManager_Free(LRUCacheManager *lruCacheManager)
{
  // destroty LRU qeueu
  LRUQueue_Free(lruCacheManager->queue);
  // free memory
  rm_free(lruCacheManager);
}


CacheData *evictFromCache(LRUCacheManager *lruCacheManager)
{
  // dequeue from LRU queue
  return (CacheData *)dequeue(lruCacheManager->queue);
}

CacheData *addToCache(LRUCacheManager *lruCacheManager, unsigned long long const hashKey, void *cacheValue)
{
  // enqueue and generate new cache entry (LRU node)
  LRUNode *newNode = enqueue(lruCacheManager->queue, hashKey, cacheValue);
  // return cache entry
  return (CacheData *)newNode;
}

void increaseImportance(LRUCacheManager *cacheManager, CacheData *cacheData)
{
  // move entry to the head of LRU queue (hot entry)
  return moveToHead(cacheManager->queue, (LRUNode *) cacheData);
}

void removeCacheData(LRUCacheManager *cacheManager, CacheData *cacheData)
{
  removeFromQueue(cacheManager->queue, (LRUNode *)cacheData);
}

bool isCacheFull(LRUCacheManager *lruCacheManager)
{
  return isFullQueue(lruCacheManager->queue);
}

void invalidateCache(LRUCacheManager *lruCacheManager){
  // empty LRU queue
  emptyQueue(lruCacheManager->queue);
}


