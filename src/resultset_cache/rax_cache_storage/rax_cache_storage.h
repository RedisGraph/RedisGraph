#ifndef RAX_CACHE_STORAGE_H
#define RAX_CACHE_STORAGE_H
#include "../result_set_cache_includes.h"
#include "rax/rax.h"

typedef struct RaxCacheStorage
{
    rax *rt;
} RaxCacheStorage;

RaxCacheStorage *RaxCacheStorage_New();
void insertToCache(RaxCacheStorage *raxCacheStorage, char *hashKey, ResultSet *resultSet);
void removeFromCache(RaxCacheStorage *raxCacheStorage, char *hashKey);
ResultSet *getFromCache(RaxCacheStorage *raxCacheStorage, char *hashKey);
void RaxCacheStorage_Free(RaxCacheStorage *raxCacheStorage);

#endif