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
#include "../../src/resultset_cache/xxhash_query_hash/xxhash_query_hash.h"
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
  // [ 2 | 4 | 3 ]
  cacheManager->increaseImportance(cacheData2, cacheManager);

  // [ 3 | 2 | 4 ]
  cacheManager->increaseImportance(cacheData3, cacheManager);
  ASSERT_TRUE(cacheManager->isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = cacheManager->evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey4, evicted->hashKey);

  cacheManager->cacheManager_free(cacheManager);
}

TEST_F(CacheManagerTest, LRUCacheManagerAndXXHASHTest){
  CacheManager *cacheManager = LRUCacheManager_New(3);
  ASSERT_TRUE(cacheManager);
  CacheData *evicted = NULL;
  char hashKey1[8] = "" ;
  char hashKey2[8] = "" ;
  char hashKey3[8] = "" ;
  char hashKey4[8] = "" ;

  QueryHash *queryHash = XXHashQueryHash_New();
  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";
  char *query3 = "MATCH (c) RETURN c";
  char *query4 = "MATCH (d) RETURN d";

  queryHash->hashQuery(query1, strlen(query1), hashKey1);
  queryHash->hashQuery(query2, strlen(query2), hashKey2);
  queryHash->hashQuery(query3, strlen(query3), hashKey3);
  queryHash->hashQuery(query4, strlen(query4), hashKey4);
  // [ | | ]
  ASSERT_FALSE(cacheManager->isCacheFull(cacheManager));
  // [ 1 | | ]
  CacheData *cacheData1 =
      cacheManager->addToCache(hashKey1, cacheManager);
  ASSERT_EQ(0, memcmp(hashKey1, cacheData1->hashKey, 8));
  ASSERT_FALSE(cacheManager->isCacheFull(cacheManager));

  // [ 2 | 1 | ]
  CacheData *cacheData2 = cacheManager->addToCache(hashKey2, cacheManager);
  ASSERT_EQ(0, memcmp(hashKey2, cacheData2->hashKey, 8));
  ASSERT_FALSE(cacheManager->isCacheFull(cacheManager));

  // [ 3 | 2 | 1 ]
  CacheData *cacheData3 = cacheManager->addToCache(hashKey3, cacheManager);
  ASSERT_EQ(0, memcmp(hashKey3, cacheData3->hashKey, 8));
  ASSERT_TRUE(cacheManager->isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = cacheManager->evictFromCache(cacheManager);
  ASSERT_EQ(0, memcmp(hashKey1, evicted->hashKey, 8));

  // [ 4 | 3 | 2 ]
  CacheData *cacheData4 = cacheManager->addToCache(hashKey4, cacheManager);
  ASSERT_EQ(0, memcmp(hashKey4, cacheData4->hashKey, 8));
  // [ 2 | 4 | 3 ]
  cacheManager->increaseImportance(cacheData2, cacheManager);

  // [ 3 | 2 | 4 ]
  cacheManager->increaseImportance(cacheData3, cacheManager);
  ASSERT_TRUE(cacheManager->isCacheFull(cacheManager));
  // [ 3 | 2 | ]
  evicted = cacheManager->evictFromCache(cacheManager);
  ASSERT_EQ(0, memcmp(hashKey4, evicted->hashKey, 8));

  queryHash->queryHash_Free(queryHash);
  cacheManager->cacheManager_free(cacheManager);
}