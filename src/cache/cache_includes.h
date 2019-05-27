/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include<stdlib.h>
#include<stdbool.h>
#include "../util/rmalloc.h"

#define HASH_KEY_LENGTH 8

typedef unsigned long long hash_key_t;

typedef void (*cacheValueFreeFunc)(void *);

/**
 * @brief  struct for holding cache data
 */
typedef struct CacheData
{
  hash_key_t hashKey;               // CacheData key - 64 bit hashed key
  void *cacheValue;                 // Value to be stored in cache
  bool isDirty;                     // Indication for written entry, for memeory release
} CacheData;