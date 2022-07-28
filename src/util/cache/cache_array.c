/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache_array.h"
#include "../rmalloc.h"
#include "../../RG.h"

// returns a pointer to the entry in the cache array with the minimum LRU
CacheEntry *CacheArray_FindMinLRU
(
	CacheEntry *cache_arr,  // list of cached items
	uint cap                // number of elements in list
) {
	ASSERT(cache_arr != NULL);

	CacheEntry *min_LRU_entry = cache_arr;

	for(size_t i = 1; i < cap; i++) {
		CacheEntry *current_entry = cache_arr + i;
		if(current_entry->LRU < min_LRU_entry->LRU) {
			min_LRU_entry = current_entry;
		}
	}

	return min_LRU_entry;
}

// assign new values to the fields of a cache entry
void CacheArray_PopulateEntry
(
	CacheEntry *entry,  // entry to set
	long long counter,  // time of update
	char *key,          // key associated with entry
	void *value         // value associated with entry
) {
	ASSERT(key   != NULL);
	ASSERT(entry != NULL);

	entry->key   = key;
	entry->value = value;
	entry->LRU   = counter;
}

// free the fields of a cache entry to prepare it for reuse
void CacheArray_ClearEntry
(
	CacheEntry *entry,             // entry to clear
	CacheEntryFreeFunc free_entry  // free callback
) {
	ASSERT(entry      != NULL);
	ASSERT(free_entry != NULL);

	if(entry->key != NULL) {
		rm_free(entry->key);
		entry->key = NULL;
	}

	if(entry->value != NULL) {
		free_entry(entry->value);
		entry->value = NULL;
	}

	entry->LRU = 0;
}

