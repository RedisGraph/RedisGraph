#ifndef CACHE_DATA_H
#define CACHE_DATA_H
#include "./result_set_cache_utils.h"

typedef struct CacheData CacheData;
typedef void (*cacheDataFreeFn)(CacheData *);
struct CacheData
{
  char hashKey[HASH_KEY_LENGTH];
  cacheDataFreeFn cacheData_Free;
};

CacheData *CacheData_New(const char *hashKey);
void CacheData_Free(CacheData *cacheData);
CacheData *CacheData_Clone(CacheData *orig);

#endif