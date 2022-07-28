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

static int free_count = 0;  // count how many cache objects been freed

typedef struct {
	const char *str;
} CacheObj;

CacheObj *CacheObj_New(const char *str) {
	CacheObj *obj = (CacheObj *)rm_malloc(sizeof(CacheObj));
	obj->str = str;
	return obj;
}

CacheObj *CacheObj_Dup(const CacheObj *obj) {
	CacheObj *dup = (CacheObj *)rm_malloc(sizeof(CacheObj));
	memcpy(dup, obj, sizeof(CacheObj));
	return dup;
}

bool CacheObj_EQ(const CacheObj *a, const CacheObj *b) {
	if(a == b) return true;
	return (strcmp(a->str, b->str) == 0);
}

void CacheObj_Free(CacheObj *obj) {
	free_count++;
	rm_free(obj);
}

TEST_F(CacheTest, ExecutionPlanCache) {
	// build a cache of strings in this case for simplicity
	Cache *cache = Cache_New(3, (CacheEntryFreeFunc)CacheObj_Free,
			(CacheEntryCopyFunc)CacheObj_Dup);

	CacheObj *item1 = CacheObj_New("1");
	CacheObj *item2 = CacheObj_New("2");
	CacheObj *item3 = CacheObj_New("3");
	CacheObj *item4 = CacheObj_New("4");

	const char *key1 = "MATCH (a) RETURN a";
	const char *key2 = "MATCH (b) RETURN b";
	const char *key3 = "MATCH (c) RETURN c";
	const char *key4 = "MATCH (d) RETURN d";

	//--------------------------------------------------------------------------
	// check for not existing key
	//--------------------------------------------------------------------------

	ASSERT_FALSE(Cache_GetValue(cache, "None existing"));

	//--------------------------------------------------------------------------
	// Set Get single item
	//--------------------------------------------------------------------------

	CacheObj *from_cache = NULL;
	Cache_SetValue(cache, key1, item1);
	from_cache = (CacheObj*)Cache_GetValue(cache, key1);
	ASSERT_TRUE(CacheObj_EQ(item1, from_cache));
	CacheObj_Free(from_cache);

	//--------------------------------------------------------------------------
	// set multiple items
	//--------------------------------------------------------------------------

	CacheObj* to_cache = (CacheObj*)Cache_SetGetValue(cache, key2, item2);
	from_cache = (CacheObj*)Cache_GetValue(cache, key2);
	ASSERT_TRUE(CacheObj_EQ(item2, from_cache));
	CacheObj_Free(to_cache);
	CacheObj_Free(from_cache);

	// fill up cache
	to_cache = (CacheObj*)Cache_SetGetValue(cache, key3, item3);
	CacheObj_Free(to_cache);
	to_cache = (CacheObj*)Cache_SetGetValue(cache, key4, item4);
	CacheObj_Free(to_cache);

	// verify that oldest entry do not exists - queue is [ 4 | 3 | 2 ]
	ASSERT_TRUE(Cache_GetValue(cache, key1) == NULL);

	Cache_Free(cache);

	// expecting CacheObjFree to be called 9 times
	ASSERT_EQ(free_count, 9);
}

TEST_F(CacheTest, ClearCache) {
	// build a cache of strings in this case for simplicity
	Cache *cache = Cache_New(5, (CacheEntryFreeFunc)CacheObj_Free,
			(CacheEntryCopyFunc)CacheObj_Dup);

	CacheObj *item1 = CacheObj_New("1");
	CacheObj *item2 = CacheObj_New("2");
	CacheObj *item3 = CacheObj_New("3");
	CacheObj *item4 = CacheObj_New("4");

	//--------------------------------------------------------------------------
	// populate cache
	//--------------------------------------------------------------------------

	Cache_SetValue(cache, "Key1", item1);
	Cache_SetValue(cache, "Key2", item2);
	Cache_SetValue(cache, "Key3", item3);
	Cache_SetValue(cache, "Key4", item4);

	// verify items are stored in cache
	ASSERT_TRUE(Cache_GetValue(cache, "Key1") != NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key2") != NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key3") != NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key4") != NULL);

	//--------------------------------------------------------------------------
	// clear cache
	//--------------------------------------------------------------------------

	Cache_Clear(cache);

	// verify cache is empty
	ASSERT_TRUE(Cache_GetValue(cache, "Key1") == NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key2") == NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key3") == NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key4") == NULL);

	// make sure we're able to re-populate the cache

	//--------------------------------------------------------------------------
	// re-populate cache
	//--------------------------------------------------------------------------

	item1 = CacheObj_New("1");
	item2 = CacheObj_New("2");
	item3 = CacheObj_New("3");
	item4 = CacheObj_New("4");

	Cache_SetValue(cache, "Key1", item1);
	Cache_SetValue(cache, "Key2", item2);
	Cache_SetValue(cache, "Key3", item3);
	Cache_SetValue(cache, "Key4", item4);

	// verify items are stored in cache
	ASSERT_TRUE(Cache_GetValue(cache, "Key1") != NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key2") != NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key3") != NULL);
	ASSERT_TRUE(Cache_GetValue(cache, "Key4") != NULL);

	Cache_Free(cache);
}

