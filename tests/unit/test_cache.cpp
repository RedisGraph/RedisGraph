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
	// Build a cache of strings in this case for simplicity.
	Cache *cache = Cache_New(3, (CacheEntryFreeFunc)rm_free, (CacheEntryCopyFunc)rm_strdup);

	char *item1 = rm_strdup("1");
	char *item2 = rm_strdup("2");
	char *item3 = rm_strdup("3");
	char *item4 = rm_strdup("4");

	char *query1 = rm_strdup("MATCH (a) RETURN a");
	char *query2 = rm_strdup("MATCH (b) RETURN b");
	char *query3 = rm_strdup("MATCH (c) RETURN c");
	char *query4 = rm_strdup("MATCH (d) RETURN d");

	// Check for not existing key.
	ASSERT_FALSE(Cache_GetValue(cache, query1));

	// Add single entry
	char *to_cache = (char *)Cache_SetGetValue(cache, query1, item1);
	char *from_cache = (char *)Cache_GetValue(cache, query1);
	ASSERT_EQ(*item1, *from_cache);
	rm_free(from_cache);
	rm_free(to_cache);

	// Add multiple entries.
	to_cache = (char *)Cache_SetGetValue(cache, query2, item2);
	from_cache = (char *)Cache_GetValue(cache, query2);
	ASSERT_EQ(*item2, *from_cache);
	rm_free(from_cache);
	rm_free(to_cache);
	to_cache = (char *)Cache_SetGetValue(cache, query3, item3);
	rm_free(to_cache);
	to_cache = (char *)Cache_SetGetValue(cache, query4, item4);
	rm_free(to_cache);

	// Verify that oldest entry do not exists - queue is [ 4 | 3 | 2 ].
	ASSERT_FALSE(Cache_GetValue(cache, query1));

	Cache_Free(cache);
}

