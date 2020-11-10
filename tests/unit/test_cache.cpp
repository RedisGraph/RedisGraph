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

static int free_count = 0;

typedef struct {
	bool free;
	const char *str;
} CacheObj;

void InitCacheObj(CacheObj *obj, const char *str) {
	obj->free = false;
	obj->str = str;
}

void CacheObjFree(CacheObj *obj) {
	free_count++;
	rm_free(obj);
}

CacheObj *CacheObjDup(const CacheObj *obj) {
	CacheObj *dup = rm_malloc(sizeof(CacheObj));
	memcpy(dup, obj, sizeof(CacheObj));
	return dup;
}

bool CacheObjEQ(const CacheObj *a, const CacheObj *b) {
	if(a == b) return true;
	return ( (a->free == b->free) && strcmp(a->str, b->str) == 0);
}	

TEST_F(CacheTest, ExecutionPlanCache) {
	// Build a cache of strings in this case for simplicity.
	Cache *cache = Cache_New(3, (CacheEntryFreeFunc)CacheObjFree,
			(CacheEntryCopyFunc)CacheObjDup);

	CacheObj item1;
	CacheObj item2;
	CacheObj item3;
	CacheObj item4;

	InitCacheObj(&item1, "1");
	InitCacheObj(&item2, "2");
	InitCacheObj(&item3, "3");
	InitCacheObj(&item4, "4");

	const char *key1 = "MATCH (a) RETURN a";
	const char *key2 = "MATCH (b) RETURN b";
	const char *key3 = "MATCH (c) RETURN c";
	const char *key4 = "MATCH (d) RETURN d";

	// Check for not existing key.
	ASSERT_FALSE(Cache_GetValue(cache, "None existing"));

	//--------------------------------------------------------------------------
	// Set Get single item
	//--------------------------------------------------------------------------
	CacheObj *from_cache = NULL;
	Cache_Set(cache, key1, &item1);
	from_cache = (CacheObj*)Cache_GetValue(cache, key1);
	ASSERT_EQ(CacheObjEQ(&item1, from_cache));
	CacheObjFree(from_cache);

	//--------------------------------------------------------------------------
	// Set multiple items
	//--------------------------------------------------------------------------

	to_cache = (CacheObj*)Cache_SetGetValue(cache, key2, &item2);
	from_cache = (CacheObj*)Cache_GetValue(cache, query2);
	ASSERT_TRUE(CacheObjEQ(&item2, from_cache));
	CacheObjFree(to_cache);
	CacheObjFree(from_cache);

	// Fill up cache
	to_cache = (CacheObj*)Cache_SetGetValue(cache, key3, &item3);
	CacheObjFree(to_cache);
	to_cache = (CacheObj*)Cache_SetGetValue(cache, key4, &item4);
	CacheObjFree(to_cache);

	// Verify that oldest entry do not exists - queue is [ 4 | 3 | 2 ].
	ASSERT_EQ(Cache_GetValue(cache, key1), NULL);

	Cache_Free(cache);

	// Expecting CacheObjFree to be called 9 times.
	ASSERT_EQ(free_count, 9);
}

