#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
#include "./cache_data.h"
#include <stdbool.h>
//pre decleration
struct CacheManager;

typedef CacheData *(*addToCacheFn)(const char *hashKey, struct CacheManager *cacheManager);
typedef void (*increaseImportanceFn)(void *cacheData, struct CacheManager *cacheManager);
typedef void (*CacheManagerFreeFn)(struct CacheManager *cacheManager);
typedef bool (*isCacheFullFn)(struct CacheManager *cacheManager);
typedef CacheData *(*evictFromCacheFn)(struct CacheManager *cacheManager);
typedef struct CacheManager
{
    isCacheFullFn isCacheFull;
    evictFromCacheFn evictFromCache;
    addToCacheFn addToCache;
    increaseImportanceFn increaseImportance;
    CacheManagerFreeFn cacheManager_free;

} CacheManager;

#endif