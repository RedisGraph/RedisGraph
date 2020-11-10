/*
 * Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache.h"
#include "../../RG.h"
#include <pthread.h>
#include <stdatomic.h>
#include "../rmalloc.h"
#include "cache_array.h"

Cache *Cache_New(uint cap, CacheEntryFreeFunc freeFunc, CacheEntryCopyFunc copyFunc) {
	ASSERT(cap > 0);
	ASSERT(copyFunc != NULL);

	Cache *cache = rm_malloc(sizeof(Cache));
	cache->size = 0;
	cache->cap = cap;
	cache->counter = 0;             // Initialize counter to zero.
	cache->lookup = raxNew();       // Instantiate key entry mapping.
	cache->copy_item = copyFunc;
	cache->free_item = freeFunc;
	cache->arr = rm_calloc(cap, sizeof(CacheEntry)); // Array of cached values.

	// Initialize the read-write lock to protect access to the cache.
	int res = pthread_rwlock_init(&cache->_cache_rwlock, NULL);
	ASSERT(res == 0);

	return cache;
}

void *Cache_GetValue(Cache *cache, const char *key) {
	void *item = NULL;

	ASSERT(cache != NULL);
	ASSERT(key != NULL);

	int res = pthread_rwlock_rdlock(&cache->_cache_rwlock);
	ASSERT(res == 0);

	CacheEntry *entry = raxFind(cache->lookup, (unsigned char *)key, strlen(key));
	if(entry == raxNotFound) goto cleanup;

	// Element is now the most recently used; update its LRU.
	// Multiple threads can be here simultaneously, use atomic updates.
	long long curr_counter = atomic_load(&cache->counter);
	long long curr_LRU = atomic_load(&entry->LRU);
	atomic_compare_exchange_strong(&entry->LRU, &curr_LRU, curr_counter);
	atomic_fetch_add(&cache->counter, 1);
	item = cache->copy_item(entry->value);

cleanup:
	res = pthread_rwlock_unlock(&cache->_cache_rwlock);
	ASSERT(res == 0);
	return item;
}

void *Cache_SetValue(Cache *cache, const char *key, void *value) {
	// TODO: Implement
	ASSERT(false);
}

void *Cache_SetGetValue(Cache *cache, const char *key, void *value) {
	ASSERT(cache != NULL);
	ASSERT(key != NULL);
	ASSERT(value != NULL);

	size_t key_len = strlen(key);
	void *value_to_return = value;

	// Acquire WRITE lock
	res = pthread_rwlock_wrlock(&cache->_cache_rwlock);
	ASSERT(res == 0);

	/* In case that another working thread had already inserted the item to the
	 * cache, no need to re-insert it. */
	CacheEntry *entry = raxFind(cache->lookup, (unsigned char *)key, key_len);
	if(entry != raxNotFound) goto cleanup;

	// key is not in cache! test to see if we cache is full?
	if(cache->size == cache->cap) {
		ASSERT("Implement");
		entry = _CacheEvictLRU(cache);

		/* The cache is full, evict the least-recently-used element
		 * and reuse its space for the new element. */
		entry = CacheArray_FindMinLRU(cache->arr, cache->cap)
		// Remove evicted element from the rax.
		raxRemove(cache->lookup, (unsigned  char *)entry->key,
				strlen(entry->key), NULL)
		CacheArray_CleanEntry(entry, cache->free_item)
	} else {
		// The array has space left in it, use the next available entry.
		entry = cache->arr + cache->size++;
	}

	// Populate the entry.
	char *k = rm_strdup(key);
	CacheArray_PopulateEntry(cache->counter, entry, k, value);
	atomic_fetch_add(&cache->counter, 1);

	// Add the new entry to the rax.
	raxInsert(cache->lookup, (unsigned char *)key, key_len, entry, NULL);
	value_to_return = cache->copy_item(value);

cleanup:
	res = pthread_rwlock_unlock(&cache->_cache_rwlock);
	ASSERT(res == 0);

	return value_to_return;
}

void Cache_Free(Cache *cache) {
	ASSERT(cache != NULL);

	// free cache entries
	for(size_t i = 0; i < cache->size; i++) {
		CacheEntry *entry = cache->arr + i;
		rm_free(entry->key);
		cache->free_item(entry->value);
	}

	rm_free(cache->arr);
	raxFree(cache->lookup);
	int res = pthread_rwlock_destroy(&cache->_cache_rwlock);
	ASSERT(res == 0);

	rm_free(cache);
}

