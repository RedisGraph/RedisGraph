#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <omp.h>
#include <iostream>
#include "../../src/util/rmalloc.h"
#include "../../src/util/cache/cache.h"
#include "../../src/execution_plan/execution_plan.h"
#ifdef __cplusplus
}
#endif

class CacheConcurrentTest:
  public ::testing::Test {
protected:
	static void SetUpTestCase() { // Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(CacheConcurrentTest, CacheConcurrency) {

	// Use rm_free since we do not insert actual EP to the cache.
	Cache *cache = Cache_New(2, (CacheEntryFreeFunc)rm_free);
	int expected_counter = 0;

	ExecutionPlan *ep1 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ExecutionPlan *ep2 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ExecutionPlan *ep3 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));

	char *query1 = (char *)"CREATE (a)";
	char *query2 = (char *)"MATCH (b) RETURN b";
	char *query3 = (char *)"MATCH (c) RETURN c";

	ASSERT_EQ(0, cache->size);

	Cache_SetValue(cache, query1, ep1);
	Cache_SetValue(cache, query2, ep2);
	expected_counter += 2;

#define NUM_THREADS 8
	omp_set_num_threads(NUM_THREADS);
	int nthreads;
	ExecutionPlan *ep;

# pragma omp parallel
{
	Cache_GetValue(cache, query1);
	Cache_GetValue(cache, query2);
	nthreads = omp_get_num_threads();

}
	expected_counter += 2*nthreads;
	// Confirm cache counter atomic increment and LRU update for the query in it.
	ASSERT_EQ(2, cache->size);
	ASSERT_EQ(expected_counter, cache->counter);
	ASSERT_TRUE(cache->arr[0].LRU < cache->arr[1].LRU);

# pragma omp parallel
{
	// Query 3 is replacing query 1 at the first position of the cache array
	Cache_SetValue(cache, query3, ep3);
	nthreads = omp_get_num_threads();
}
	// The first thread inserts Q3 and updates the counter.
	// The rest of the threads find Q3 in the cache and finish.
	expected_counter++;
	ASSERT_EQ(expected_counter, cache->counter);
	ASSERT_EQ(expected_counter-1, cache->arr[0].LRU);

	// Test parallel reading and writing to the cache with evictions
# pragma omp parallel
{
	int id = omp_get_thread_num();
	ep1 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ep2 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ep3 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	if (id%2) {
		Cache_SetValue(cache, query1, ep1);
		Cache_SetValue(cache, query2, ep2);
		Cache_SetValue(cache, query3, ep3);
		Cache_GetValue(cache, query1);
	} else {
		Cache_SetValue(cache, query3, ep3);
		Cache_SetValue(cache, query2, ep2);
		Cache_SetValue(cache, query1, ep1);
		Cache_GetValue(cache, query3);
	}
}
Cache_Free(cache);
}
