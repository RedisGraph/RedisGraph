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

#include "../../src/cache/cache.h"
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
  Cache *resultSetCache = cacheNew(3, (cacheValueFreeFunc)ResultSet_Free);

  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  char *query1 = "MATCH (a) RETURN a";
  char *query2 = "MATCH (b) RETURN b";
  char *query3 = "MATCH (c) RETURN c";
  char *query4 = "MATCH (d) RETURN d";

  //check for not existing key
  ASSERT_FALSE(cacheGetValue(resultSetCache, query1, strlen(query1)));

  // add single entry
  cacheSetValue(resultSetCache, query1, strlen(query1), rs1);
  ASSERT_EQ(rs1, cacheGetValue(resultSetCache, query1, strlen(query1)));

  // add multiple entries
  cacheSetValue(resultSetCache, query2, strlen(query2), rs2);
  ASSERT_EQ(rs2, cacheGetValue(resultSetCache, query2, strlen(query2)));
  cacheSetValue(resultSetCache, query3, strlen(query3), rs3);
  ASSERT_EQ(rs3, cacheGetValue(resultSetCache, query3, strlen(query3)));
  cacheSetValue(resultSetCache, query4, strlen(query4), rs4);
  ASSERT_EQ(rs4, cacheGetValue(resultSetCache, query4, strlen(query4)));

  //verify that oldest entry is not exists - queue is [ 4 | 3 | 2 ]
  ASSERT_FALSE(cacheGetValue(resultSetCache, query1, strlen(query1)));

  //clear cache
  cacheClear(resultSetCache);
  ASSERT_FALSE(cacheGetValue(resultSetCache, query1, strlen(query1)));
  ASSERT_FALSE(cacheGetValue(resultSetCache, query2, strlen(query2)));
  ASSERT_FALSE(cacheGetValue(resultSetCache, query3, strlen(query3)));
  ASSERT_FALSE(cacheGetValue(resultSetCache, query4, strlen(query4)));

  //re-alloc since they are delted or will be deleted in the cache, rs4 will be deleted once those will be inserted
  rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));

  cacheSetValue(resultSetCache, query1, strlen(query1), rs1);
  ASSERT_EQ(rs1, cacheGetValue(resultSetCache, query1, strlen(query1)));
  cacheSetValue(resultSetCache, query2, strlen(query2), rs2);
  ASSERT_EQ(rs2, cacheGetValue(resultSetCache, query2, strlen(query2)));
  cacheSetValue(resultSetCache, query3, strlen(query3), rs3);
  ASSERT_EQ(rs3, cacheGetValue(resultSetCache, query3, strlen(query3)));

  ASSERT_FALSE(cacheGetValue(resultSetCache, query4, strlen(query4)));

  cacheFree(resultSetCache);
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


void ResultSetPtr_Free(ResultSet** ptr){
  ResultSet_Free(*ptr);
}
TEST_F(LRUQueueTest, TestLRUQueue){
  LRUQueue *queue = lruQueueNew(10, sizeof(ResultSet *), (lruDataFreeFunc)ResultSetPtr_Free);

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
  void *data0 = lruQueueEnqueue(queue, &rs0);
  
  ResultSet *rs1 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void* data1 = lruQueueEnqueue(queue, &rs1);
 
  ResultSet *rs2 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void* data2 = lruQueueEnqueue(queue, &rs2);

  ResultSet *rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void *data3 = lruQueueEnqueue(queue, &rs3);

  ResultSet *rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void *data4 = lruQueueEnqueue(queue, &rs4);

  ResultSet *rs5 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void *data5 = lruQueueEnqueue(queue, &rs5);

  ResultSet *rs6 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void *data6 = lruQueueEnqueue(queue, &rs6);

  ResultSet *rs7 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void *data7 = lruQueueEnqueue(queue, &rs7);

  ASSERT_EQ(0, memcmp(&rs0, (ResultSet **)data0, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs1, (ResultSet **)data1, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs2, (ResultSet **)data2, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs3, (ResultSet **)data3, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs4, (ResultSet **)data4, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs5, (ResultSet **)data5, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs6, (ResultSet **)data6, sizeof(ResultSet *)));
  ASSERT_EQ(0, memcmp(&rs7, (ResultSet **)data7, sizeof(ResultSet *)));

  lruQueueRemoveFromQueue(queue, data6);
  lruQueueRemoveFromQueue(queue, data4);
  lruQueueRemoveFromQueue(queue, data3);

  rs3 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  data3 = lruQueueEnqueue(queue, &rs3);
  ASSERT_EQ(0, memcmp(&rs3, (ResultSet **)data3, sizeof(ResultSet *)));

  rs4 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  data4 = lruQueueEnqueue(queue, &rs4);
  ASSERT_EQ(0, memcmp(&rs4, (ResultSet **)data4, sizeof(ResultSet *)));

  rs6 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  data6 = lruQueueEnqueue(queue, &rs6);
  ASSERT_EQ(0, memcmp(&rs6, (ResultSet **)data6, sizeof(ResultSet *)));

  ResultSet *rs8 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void* data8 = lruQueueEnqueue(queue, &rs8);
  ASSERT_EQ(0, memcmp(&rs8, (ResultSet **)data8, sizeof(ResultSet *)));

  ResultSet *rs9 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  void* data9 = lruQueueEnqueue(queue, &rs9);
  ASSERT_EQ(0, memcmp(&rs9, (ResultSet **)data9, sizeof(ResultSet *)));

  ResultSet *rs10 = (ResultSet *)rm_calloc(1, sizeof(ResultSet));
  lruQueueDequeue(queue);
  void* data10 = lruQueueEnqueue(queue, &rs10);
  ASSERT_EQ(0, memcmp(&rs10, (ResultSet **)data10, sizeof(ResultSet *)));

  lruQueueFree(queue);
}
