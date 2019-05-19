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
#include "../../src/resultset_cache/rax_cache_storage/rax_cache_storage.h"
#include "../../src/util/rmalloc.h"
#include "../../deps/rax/rax.h"
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

class CacheStorageTest : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  { // Use the malloc family for allocations
    Alloc_Reset();
  }
};

TEST_F(CacheManagerTest, LRUCacheManagerTest) {

  LRUCacheManager *cacheManager = LRUCacheManager_New(3);
  ASSERT_TRUE(cacheManager);
  CacheData *evicted = NULL;
  char hashKey1[8] = "1234567";
  char hashKey2[8] = "2345678";
  char hashKey3[8] = "3456789";
  char hashKey4[8] = "4567890";

  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  // [ | | ]
  ASSERT_FALSE(isCacheFull(cacheManager));
  // [ 1 | | ]
  CacheData *cacheData1 =
      addToCache(cacheManager, hashKey1, rs1);
  ASSERT_STREQ(hashKey1, cacheData1->hashKey);
  ASSERT_EQ(rs1, cacheData1->resultSet);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 2 | 1 | ]
  CacheData *cacheData2 = addToCache(cacheManager, hashKey2, rs2);
  ASSERT_STREQ(hashKey2, cacheData2->hashKey);
  ASSERT_EQ(rs2, cacheData2->resultSet);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 3 | 2 | 1 ]
  CacheData *cacheData3 = addToCache(cacheManager, hashKey3, rs3);
  ASSERT_STREQ(hashKey3, cacheData3->hashKey);
  ASSERT_EQ(rs3, cacheData3->resultSet);
  ASSERT_TRUE(isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey1, evicted->hashKey);

  // [ 4 | 3 | 2 ]
  CacheData *cacheData4 = addToCache(cacheManager, hashKey4, rs4);
  ASSERT_EQ(rs4, cacheData4->resultSet);
  ASSERT_STREQ(hashKey4, cacheData4->hashKey);
  // [ 2 | 4 | 3 ]
  increaseImportance(cacheManager, cacheData2);

  // [ 3 | 2 | 4 ]
  increaseImportance(cacheManager, cacheData3);
  ASSERT_TRUE(isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey4, evicted->hashKey);
  ASSERT_EQ(rs4, cacheData4->resultSet);

  LRUCacheManager_Free(cacheManager);
}

TEST_F(CacheManagerTest, LRUCacheManagerAndXXHASHTest){
  LRUCacheManager *cacheManager = LRUCacheManager_New(3);
  ASSERT_TRUE(cacheManager);
  CacheData *evicted = NULL;
  char hashKey1[8] = "" ;
  char hashKey2[8] = "" ;
  char hashKey3[8] = "" ;
  char hashKey4[8] = "" ;

  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";
  char *query3 = "MATCH (c) RETURN c";
  char *query4 = "MATCH (d) RETURN d";

  hashQuery(query1, strlen(query1), hashKey1);
  hashQuery(query2, strlen(query2), hashKey2);
  hashQuery(query3, strlen(query3), hashKey3);
  hashQuery(query4, strlen(query4), hashKey4);
  // [ | | ]
  ASSERT_FALSE(isCacheFull(cacheManager));
  // [ 1 | | ]
  CacheData *cacheData1 =
      addToCache(cacheManager, hashKey1, rs1);
  ASSERT_EQ(0, memcmp(hashKey1, cacheData1->hashKey, 8));
  ASSERT_EQ(rs1, cacheData1->resultSet);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 2 | 1 | ]
  CacheData *cacheData2 = addToCache(cacheManager, hashKey2, rs2);
  ASSERT_EQ(0, memcmp(hashKey2, cacheData2->hashKey, 8));
  ASSERT_EQ(rs2, cacheData2->resultSet);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 3 | 2 | 1 ]
  CacheData *cacheData3 = addToCache(cacheManager, hashKey3, rs3);
  ASSERT_EQ(0, memcmp(hashKey3, cacheData3->hashKey, 8));
  ASSERT_EQ(rs3, cacheData3->resultSet);
  ASSERT_TRUE(isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_EQ(0, memcmp(hashKey1, cacheData1->hashKey, 8));

  // [ 4 | 3 | 2 ]
  CacheData *cacheData4 = addToCache(cacheManager, hashKey4, rs4);
  ASSERT_EQ(0, memcmp(hashKey4, cacheData4->hashKey, 8));
  ASSERT_EQ(rs4, cacheData4->resultSet);
  // [ 2 | 4 | 3 ]
  increaseImportance(cacheManager, cacheData2);

  // [ 3 | 2 | 4 ]
  increaseImportance(cacheManager, cacheData3);
  ASSERT_TRUE(isCacheFull(cacheManager));
  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_EQ(0, memcmp(hashKey4, evicted->hashKey, 8));
  ASSERT_EQ(rs4, cacheData4->resultSet);

  LRUCacheManager_Free(cacheManager);
}

TEST_F(CacheStorageTest, RaxTestEdgeCase){

  char normalKey[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
  char nullKey[8] = {'a', 'b', 'c', '\0', 'a', 'b', 'c', 'd'};
  char nullKey2[8] = {'a', 'b', 'c', '\0', 'a', 'b', 'c', 'e'};

  ResultSet rs1;
  ResultSet rs2;
  ResultSet rs3;

  rax *rt = raxNew();
  raxInsert(rt, (unsigned char *)normalKey, 8, &rs1, NULL);
  ResultSet* returnedRs = (ResultSet*)raxFind(rt, (unsigned char *)normalKey,8);
  ASSERT_EQ(&rs1, returnedRs);

  raxInsert(rt, (unsigned char *)nullKey, 8, &rs2, NULL);
  raxInsert(rt, (unsigned char *)nullKey2, 8, &rs3, NULL);
  returnedRs = (ResultSet*)raxFind(rt, (unsigned char *)nullKey,8);
  ASSERT_EQ(&rs2, returnedRs);
  returnedRs = (ResultSet*)raxFind(rt, (unsigned char *)nullKey2,8);
  ASSERT_NE(&rs2, returnedRs);
  ASSERT_EQ(&rs3, returnedRs);

}
