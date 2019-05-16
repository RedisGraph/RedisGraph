#include "./cache_data.h"
#include "../util/rmalloc.h"
#include <stdlib.h>

CacheData *CacheData_New(const char *hashKey) {
  CacheData *cacheData = rm_malloc(sizeof(CacheData));
  memcpy(cacheData->hashKey, hashKey, HASH_KEY_LENGTH);
  cacheData->cacheData_Free = &CacheData_Free;
  return cacheData;
}

void CacheData_Free(CacheData *cacheData) { 
    rm_free(cacheData);
}

CacheData *CacheData_Clone(CacheData *orig){
  return CacheData_New(orig->hashKey);
}
