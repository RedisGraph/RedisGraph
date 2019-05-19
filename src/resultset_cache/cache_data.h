#ifndef CACHE_DATA_H
#define CACHE_DATA_H
#include "./result_set_cache_includes.h"

typedef struct CacheData CacheData;
struct CacheData
{
  char hashKey[HASH_KEY_LENGTH];
  ResultSet *resultSet;
  bool isDirty;
};

#endif