/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License
 * Agreement
 */

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "../../src/resultset_cache/lru_cache_manager/lru_cache_manager.h"
#include "../../src/util/rmalloc.h"
#include <stdio.h>

#ifdef __cplusplus
}
#endif

class CacheManagerTest : public ::testing::Test {
protected:
  static void SetUpTestCase() { // Use the malloc family for allocations
    Alloc_Reset();
  }
};

//tested #define RESULT_SET_CACHE_SIZE 3
TEST_F(CacheManagerTest, LRUCacheManagerTest) {

  CacheManager *cacheManager = LRUCacheManager_New(3);
  ASSERT_TRUE(cacheManager);
  CacheData *evicted = NULL;
  char hashKey1[8] = "1234567";
  char hashKey2[8] = "2345678";
  char hashKey3[8] = "3456789";
  char hashKey4[8] = "4567890";

  // [ | | ]
  ASSERT_FALSE(cacheManager->isCacheFull(cacheManager));
  // [ 1 | | ]
  CacheData *cacheData1 =
      cacheManager->addToCache(hashKey1, cacheManager);
  ASSERT_STREQ(hashKey1, cacheData1->hashKey);
  ASSERT_FALSE(cacheManager->isCacheFull(cacheManager));

  // [ 2 | 1 | ]
  CacheData *cacheData2 = cacheManager->addToCache(hashKey2, cacheManager);
  ASSERT_STREQ(hashKey2, cacheData2->hashKey);
  ASSERT_FALSE(cacheManager->isCacheFull(cacheManager));

  // [ 3 | 2 | 1 ]
  CacheData *cacheData3 = cacheManager->addToCache(hashKey3, cacheManager);
  ASSERT_STREQ(hashKey3, cacheData3->hashKey);
  ASSERT_TRUE(cacheManager->isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = cacheManager->evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey1, evicted->hashKey);

  // [ 4 | 3 | 2 ]
  CacheData *cacheData4 = cacheManager->addToCache(hashKey4, cacheManager);
  ASSERT_STREQ(hashKey4, cacheData4->hashKey);

  cacheManager->increaseImportance(cacheData2, cacheManager);
  cacheManager->increaseImportance(cacheData3, cacheManager);
  ASSERT_TRUE(cacheManager->isCacheFull(cacheManager));
  evicted = cacheManager->evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey4, evicted->hashKey);

  cacheManager->cacheManager_free(cacheManager);
}