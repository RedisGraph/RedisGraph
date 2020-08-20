#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "../../src/util/rmalloc.h"
#include "../../src/util/cache/cache.h"
#include "../../src/execution_plan/execution_plan.h"
#ifdef __cplusplus
}
#endif

class CacheTest:
	public ::testing::Test {
  protected:
	static void SetUpTestCase() { // Use the malloc family for allocations
		Alloc_Reset();
	}
};

TEST_F(CacheTest, ExecutionPlanCache) {
	Cache *cache = Cache_New(3, (CacheItemFreeFunc)ExecutionPlan_Free);

	ExecutionPlan *ep1 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ExecutionPlan *ep2 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ExecutionPlan *ep3 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));
	ExecutionPlan *ep4 = (ExecutionPlan *)rm_calloc(1, sizeof(ExecutionPlan));

	char *query1 = "MATCH (a) RETURN a";
	char *query2 = "MATCH (b) RETURN b";
	char *query3 = "MATCH (c) RETURN c";
	char *query4 = "MATCH (d) RETURN d";

	// Check for not existing key.
	ASSERT_FALSE(Cache_GetValue(cache, query1));

	// Add single entry
	Cache_SetValue(cache, query1, ep1);
	ASSERT_EQ(ep1, Cache_GetValue(cache, query1));

	// Add multiple entries.
	Cache_SetValue(cache, query2, ep2);
	ASSERT_EQ(ep2, Cache_GetValue(cache, query2));
	Cache_SetValue(cache, query3, ep3);
	ASSERT_EQ(ep3, Cache_GetValue(cache, query3));
	Cache_SetValue(cache, query4, ep4);
	ASSERT_EQ(ep4, Cache_GetValue(cache, query4));

	// Verify that oldest entry do not exists - queue is [ 4 | 3 | 2 ].
	ASSERT_FALSE(Cache_GetValue(cache, query1));

	Cache_Free(cache);
}

