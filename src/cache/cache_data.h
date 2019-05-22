/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef CACHE_DATA_H
#define CACHE_DATA_H
#include "./cache_includes.h"

/**
 * @brief  struct for holding cache data
 */
typedef struct CacheData
{
  unsigned long long hashKey;    // CacheData key - Query hashed by XXHASH
  void *cacheValue;             // Value - ResultSet
  bool isDirty;                     // Indication for written entry, for memeory release
} CacheData;

#endif