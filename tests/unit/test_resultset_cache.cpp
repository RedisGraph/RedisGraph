/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License
 * Agreement
 */

#include "../../deps/googletest/include/gtest/gtest.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../src/resultset_cache/lru_cache_manager/lru_cache_manager.h"
#include "../../src/resultset_cache/xxhash_query_hash/xxhash_query_hash.h"
#include "../../src/resultset_cache/rax_cache_storage/rax_cache_storage.h"
#include "../../src/resultset_cache/resultset_cache.h"
#include "../../src/util/rmalloc.h"
#include "../../deps/rax/rax.h"
#include <stdio.h>

#ifdef __cplusplus
}
#endif

class LRUQueueTest : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  { // Use the malloc family for allocations
    Alloc_Reset();
  }
};

class CacheManagerTest : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  { // Use the malloc family for allocations
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

class ResultSetCacheTest : public ::testing::Test
{
protected:
  static void SetUpTestCase()
  { // Use the malloc family for allocations
    Alloc_Reset();
  }
};

TEST_F(CacheManagerTest, LRUCacheManagerTest)
{

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
      addToCache(cacheManager, (unsigned long long)hashKey1, rs1);
  ASSERT_STREQ(hashKey1, (char*)cacheData1->hashKey);
  ASSERT_EQ(rs1, cacheData1->cacheValue);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 2 | 1 | ]
  CacheData *cacheData2 = addToCache(cacheManager, (unsigned long long)hashKey2, rs2);
  ASSERT_STREQ(hashKey2, (char *)cacheData2->hashKey);
  ASSERT_EQ(rs2, cacheData2->cacheValue);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 3 | 2 | 1 ]
  CacheData *cacheData3 = addToCache(cacheManager, (unsigned long long)hashKey3, rs3);
  ASSERT_STREQ(hashKey3, (char *)cacheData3->hashKey);
  ASSERT_EQ(rs3, cacheData3->cacheValue);
  ASSERT_TRUE(isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey1, (char *)evicted->hashKey);

  // [ 4 | 3 | 2 ]
  CacheData *cacheData4 = addToCache(cacheManager, (unsigned long long)hashKey4, rs4);
  ASSERT_EQ(rs4, cacheData4->cacheValue);
  ASSERT_STREQ(hashKey4, (char *)cacheData4->hashKey);
  // [ 2 | 4 | 3 ]
  increaseImportance(cacheManager, cacheData2);

  // [ 3 | 2 | 4 ]
  increaseImportance(cacheManager, cacheData3);
  ASSERT_TRUE(isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_STREQ(hashKey4, (char *)evicted->hashKey);
  ASSERT_EQ(rs4, cacheData4->cacheValue);

  LRUCacheManager_Free(cacheManager);
}

TEST_F(CacheManagerTest, LRUCacheManagerAndXXHASHTest)
{
  LRUCacheManager *cacheManager = LRUCacheManager_New(3);
  ASSERT_TRUE(cacheManager);
  CacheData *evicted = NULL;


  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";
  char *query3 = "MATCH (c) RETURN c";
  char *query4 = "MATCH (d) RETURN d";

  unsigned long long hashKey1 = hashQuery(query1, strlen(query1));
  unsigned long long hashKey2 = hashQuery(query2, strlen(query2));
  unsigned long long hashKey3 = hashQuery(query3, strlen(query3));
  unsigned long long hashKey4 = hashQuery(query4, strlen(query4));
  // [ | | ]
  ASSERT_FALSE(isCacheFull(cacheManager));
  // [ 1 | | ]
  CacheData *cacheData1 =
      addToCache(cacheManager, hashKey1, rs1);
  ASSERT_EQ(hashKey1, cacheData1->hashKey);
  ASSERT_EQ(rs1, cacheData1->cacheValue);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 2 | 1 | ]
  CacheData *cacheData2 = addToCache(cacheManager, hashKey2, rs2);
  ASSERT_EQ(hashKey2, cacheData2->hashKey);
  ASSERT_EQ(rs2, cacheData2->cacheValue);
  ASSERT_FALSE(isCacheFull(cacheManager));

  // [ 3 | 2 | 1 ]
  CacheData *cacheData3 = addToCache(cacheManager, hashKey3, rs3);
  ASSERT_EQ(hashKey3, cacheData3->hashKey);
  ASSERT_EQ(rs3, cacheData3->cacheValue);
  ASSERT_TRUE(isCacheFull(cacheManager));

  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_EQ(hashKey1, cacheData1->hashKey);

  // [ 4 | 3 | 2 ]
  CacheData *cacheData4 = addToCache(cacheManager, hashKey4, rs4);
  ASSERT_EQ(hashKey4, cacheData4->hashKey);
  ASSERT_EQ(rs4, cacheData4->cacheValue);
  // [ 2 | 4 | 3 ]
  increaseImportance(cacheManager, cacheData2);

  // [ 3 | 2 | 4 ]
  increaseImportance(cacheManager, cacheData3);
  ASSERT_TRUE(isCacheFull(cacheManager));
  // [ 3 | 2 | ]
  evicted = evictFromCache(cacheManager);
  ASSERT_EQ(hashKey4, evicted->hashKey);
  ASSERT_EQ(rs4, cacheData4->cacheValue);

  invalidateCache(cacheManager);
  ASSERT_FALSE(isCacheFull(cacheManager));
  ASSERT_TRUE(isEmptyQueue(cacheManager->queue));
  rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  cacheData1 =
      addToCache(cacheManager, hashKey1, rs1);
  ASSERT_FALSE(isCacheFull(cacheManager));
  cacheData2 = addToCache(cacheManager, hashKey2, rs2);
  ASSERT_FALSE(isCacheFull(cacheManager));
  cacheData3 = addToCache(cacheManager, hashKey3, rs3);
  ASSERT_TRUE(isCacheFull(cacheManager));

  LRUCacheManager_Free(cacheManager);
}

TEST_F(CacheStorageTest, RaxTestEdgeCase)
{

  char normalKey[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};
  char nullKey[8] = {'a', 'b', 'c', '\0', 'a', 'b', 'c', 'd'};
  char nullKey2[8] = {'a', 'b', 'c', '\0', 'a', 'b', 'c', 'e'};

  ResultSet rs1;
  ResultSet rs2;
  ResultSet rs3;

  rax *rt = raxNew();
  raxInsert(rt, (unsigned char *)normalKey, 8, &rs1, NULL);
  ResultSet *returnedRs = (ResultSet *)raxFind(rt, (unsigned char *)normalKey, 8);
  ASSERT_EQ(&rs1, returnedRs);

  raxInsert(rt, (unsigned char *)nullKey, 8, &rs2, NULL);
  raxInsert(rt, (unsigned char *)nullKey2, 8, &rs3, NULL);
  returnedRs = (ResultSet *)raxFind(rt, (unsigned char *)nullKey, 8);
  ASSERT_EQ(&rs2, returnedRs);
  returnedRs = (ResultSet *)raxFind(rt, (unsigned char *)nullKey2, 8);
  ASSERT_NE(&rs2, returnedRs);
  ASSERT_EQ(&rs3, returnedRs);
}

TEST_F(ResultSetCacheTest, ResultSetCacheTest)
{
  ResultSetCache *resultSetCache = ResultSetCache_New(3);

  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";
  char *query3 = "MATCH (c) RETURN c";
  char *query4 = "MATCH (d) RETURN d";

  //check for not existing key
  ASSERT_FALSE(getResultSet(resultSetCache, query1, strlen(query1)));

  // add single entry
  storeResultSet(resultSetCache, query1, strlen(query1), rs1);
  ASSERT_EQ(rs1, getResultSet(resultSetCache, query1, strlen(query1)));

  // add multiple entries
  storeResultSet(resultSetCache, query2, strlen(query2), rs2);
  ASSERT_EQ(rs2, getResultSet(resultSetCache, query2, strlen(query2)));
  storeResultSet(resultSetCache, query3, strlen(query3), rs3);
  ASSERT_EQ(rs3, getResultSet(resultSetCache, query3, strlen(query3)));
  storeResultSet(resultSetCache, query4, strlen(query4), rs4);
  ASSERT_EQ(rs4, getResultSet(resultSetCache, query4, strlen(query4)));

  //verify that oldest entry is not exists - queue is [ 4 | 3 | 2 ]
  ASSERT_FALSE(getResultSet(resultSetCache, query1, strlen(query1)));

  //clear cache
  clearCache(resultSetCache);
  ASSERT_FALSE(getResultSet(resultSetCache, query1, strlen(query1)));
  ASSERT_FALSE(getResultSet(resultSetCache, query2, strlen(query2)));
  ASSERT_FALSE(getResultSet(resultSetCache, query3, strlen(query3)));
  ASSERT_FALSE(getResultSet(resultSetCache, query4, strlen(query4)));

  //re-alloc since they are delted or will be deleted in the cache, rs4 will be deleted once those will be inserted
  rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  storeResultSet(resultSetCache, query1, strlen(query1), rs1);
  ASSERT_EQ(rs1, getResultSet(resultSetCache, query1, strlen(query1)));
  storeResultSet(resultSetCache, query2, strlen(query2), rs2);
  ASSERT_EQ(rs2, getResultSet(resultSetCache, query2, strlen(query2)));
  storeResultSet(resultSetCache, query3, strlen(query3), rs3);
  ASSERT_EQ(rs3, getResultSet(resultSetCache, query3, strlen(query3)));

  ASSERT_FALSE(getResultSet(resultSetCache, query4, strlen(query4)));

  ResultSetCache_Free(resultSetCache);
}

TEST_F(ResultSetCacheTest, GraphCacheTest)
{
  
  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";


  char *graph1 = "graph1";
  char *graph2 = "graph2";

  ASSERT_FALSE(graphCacheGet(graph1, query1));
  ResultSet *rs11 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query1, rs11);
  ASSERT_EQ(rs11, graphCacheGet(graph1, query1));
  ASSERT_FALSE(graphCacheGet(graph2, query1));
  ResultSet *rs21 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph2, query1, rs21);
  ASSERT_EQ(rs21, graphCacheGet(graph2, query1));
  ASSERT_NE(graphCacheGet(graph1, query1), graphCacheGet(graph2, query1));

  graphCacheMarkInvalid(graph1);
  ASSERT_FALSE(graphCacheGet(graph1, query1));
  ASSERT_EQ(rs21, graphCacheGet(graph2, query1));

  graphCacheInvalidate(graph1);
  ASSERT_FALSE(graphCacheGet(graph1, query1));
  ASSERT_EQ(rs21, graphCacheGet(graph2, query1));

  rs11 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query1, rs11);
  ASSERT_EQ(rs11, graphCacheGet(graph1, query1));
  ResultSet *rs12 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query2, rs12);
  ASSERT_EQ(rs11, graphCacheGet(graph1, query1));
  ASSERT_EQ(rs12, graphCacheGet(graph1, query2));
  ASSERT_EQ(rs21, graphCacheGet(graph2, query1));
  ASSERT_FALSE(graphCacheGet(graph2, query2));

  removeGraphCache(graph1);
  ASSERT_EQ(rs21, graphCacheGet(graph2, query1));
  removeGraphCache(graph2);
}

TEST_F(ResultSetCacheTest, GraphCacheRemoveEntryTest){
  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";
  char *query3 = "MATCH (c) RETURN c";
  char *query4 = "MATCH (d) RETURN d";
  char *query5 = "MATCH (e) RETURN e";
  char *query6 = "MATCH (f) RETURN f";
  char *query7 = "MATCH (g) RETURN g";
  char *query8 = "MATCH (h) RETURN h";

  char *graph1 = "graph1";

  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query1, rs1);
  ASSERT_EQ(rs1, graphCacheGet(graph1, query1));

  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query2, rs2);
  ASSERT_EQ(rs2, graphCacheGet(graph1, query2));

  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query3, rs3);
  ASSERT_EQ(rs3, graphCacheGet(graph1, query3));

  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query4, rs4);
  ASSERT_EQ(rs4, graphCacheGet(graph1, query4));

  ResultSet *rs5 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query5, rs5);
  ASSERT_EQ(rs5, graphCacheGet(graph1, query5));

  ResultSet *rs6 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query6, rs6);
  ASSERT_EQ(rs6, graphCacheGet(graph1, query6));

  ResultSet *rs7 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query7, rs7);
  ASSERT_EQ(rs7, graphCacheGet(graph1, query7));

  ResultSet *rs8 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query8, rs8);
  ASSERT_EQ(rs8, graphCacheGet(graph1, query8));

  graphCacheRemove(graph1, query7);
  ASSERT_FALSE(graphCacheGet(graph1, query7));

  graphCacheRemove(graph1, query4);
  ASSERT_FALSE(graphCacheGet(graph1, query4));

  graphCacheRemove(graph1, query3);
  ASSERT_FALSE(graphCacheGet(graph1, query3));

  rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query3, rs3);
  ASSERT_EQ(rs3, graphCacheGet(graph1, query3));

  rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query4, rs4);
  ASSERT_EQ(rs4, graphCacheGet(graph1, query4));

  rs7 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  graphCacheSet(graph1, query7, rs7);
  ASSERT_EQ(rs7, graphCacheGet(graph1, query7));

  removeGraphCache(graph1);
}

TEST_F(LRUQueueTest, TestLRUQueue){
  LRUQueue *queue = LRUQueue_New(10);

  unsigned long long key0 = 0;
  unsigned long long key1 = 1;
  unsigned long long key2 = 2;
  unsigned long long key3 = 3;
  unsigned long long key4 = 4;
  unsigned long long key5 = 5;
  unsigned long long key6 = 6;
  unsigned long long key7 = 7;
  unsigned long long key8 = 8;
  unsigned long long key9 = 9;
  unsigned long long key10 = 10;

  ResultSet *rs0 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key0, rs0);
  ASSERT_EQ(rs0, queue->queue[0].cacheData.cacheValue);

  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key1, rs1);
  ASSERT_EQ(rs1, queue->queue[1].cacheData.cacheValue);

  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key2, rs2);
  ASSERT_EQ(rs2, queue->queue[2].cacheData.cacheValue);

  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key3, rs3);
  ASSERT_EQ(rs3, queue->queue[3].cacheData.cacheValue);

  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key4, rs4);
  ASSERT_EQ(rs4, queue->queue[4].cacheData.cacheValue);

  ResultSet *rs5 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key5, rs5);
  ASSERT_EQ(rs5, queue->queue[5].cacheData.cacheValue);

  ResultSet *rs6 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key6, rs6);
  ASSERT_EQ(rs6, queue->queue[6].cacheData.cacheValue);

  ResultSet *rs7 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key7, rs7);
  ASSERT_EQ(rs7, queue->queue[7].cacheData.cacheValue);

  removeFromQueue(queue, &queue->queue[6]);
  removeFromQueue(queue, &queue->queue[4]);
  removeFromQueue(queue, &queue->queue[3]);

  rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key3, rs3);
  ASSERT_EQ(rs3, queue->queue[3].cacheData.cacheValue);

  rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key4, rs4);
  ASSERT_EQ(rs4, queue->queue[4].cacheData.cacheValue);

  rs6 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key6, rs6);
  ASSERT_EQ(rs6, queue->queue[6].cacheData.cacheValue);

  ResultSet *rs8 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key8, rs8);
  ASSERT_EQ(rs8, queue->queue[8].cacheData.cacheValue);

  ResultSet *rs9 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  enqueue(queue, key9, rs9);
  ASSERT_EQ(rs9, queue->queue[9].cacheData.cacheValue);

  ResultSet *rs10 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  dequeue(queue);
  enqueue(queue, key10, rs10);
  ASSERT_EQ(rs9, queue->queue[9].cacheData.cacheValue);

  LRUQueue_Free(queue);
}
