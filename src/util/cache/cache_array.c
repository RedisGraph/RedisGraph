/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache_array.h"
#include "../rmalloc.h"
#include "../../RG.h"

CacheEntry *CacheArray_FindMinLRU(CacheEntry *cache_arr, uint cap) {
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

CacheEntry *CacheArray_PopulateEntry(long long counter, CacheEntry *entry, char *key,
  									void *value) {

	entry->key   = key;
	entry->value = value;
	entry->LRU   = counter;

	return entry;
}

void CacheArray_CleanEntry(CacheEntry *entry, CacheEntryFreeFunc free_entry) {
	ASSERT(entry != NULL);
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

