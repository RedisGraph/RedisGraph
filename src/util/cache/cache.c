/*
 *Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 *This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache.h"
#include "../../RG.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include "../rmalloc.h"
#include "cache_array.h"

Cache *Cache_New(uint cap, CacheEntryFreeFunc freeFunc) {
	Cache *cache = rm_malloc(sizeof(Cache));
	cache->cap = cap;
	cache->size = 0;
	// Initialize counter to zero.
	cache->counter = 0;
	// Instantiate a new array to store cached values.
	cache->arr = rm_calloc(cap, sizeof(CacheEntry));
	cache->free_entry = freeFunc;
	// Instantiate the lookup rax for fast cache retrievals.
	cache->lookup = raxNew();
	// Initialize the read-write lock to protect access to the cache.
	assert(pthread_rwlock_init(&cache->_cache_rwlock, NULL) == 0);
	return cache;
}

inline void *Cache_GetValue(Cache *cache, const char *key) {
	pthread_rwlock_rdlock(&cache->_cache_rwlock);
	CacheEntry *entry = raxFind(cache->lookup, (unsigned char *)key, strlen(key));
	if(entry == raxNotFound) {
		pthread_rwlock_unlock(&cache->_cache_rwlock);
		return NULL;
	}
	// Element is now the most recently used; update its LRU.
	// Multiple threads can be here simultaneously, make atomic updates.
	ulong curr_counter = atomic_load(&cache->counter);
	ulong curr_LRU = atomic_load(&entry->LRU);
	atomic_compare_exchange_strong(&entry->LRU, &curr_LRU, curr_counter);
	atomic_fetch_add(&cache->counter, 1);
	pthread_rwlock_unlock(&cache->_cache_rwlock);
	return entry->value;
}

bool Cache_SetValue(Cache *cache, const char *orig_key, void *value) {
	pthread_rwlock_wrlock(&cache->_cache_rwlock);
	/* In case that another working thread had already inserted the query to the
	 * cache, no need to insert it again. */
	size_t key_len = strlen(orig_key);
	CacheEntry *entry = raxFind(cache->lookup, (unsigned char *)orig_key,
	  					key_len);
	if(entry != raxNotFound) {
		pthread_rwlock_unlock(&cache->_cache_rwlock);
		return false;
	}
	if(cache->size == cache->cap) {
		/* The cache is full, evict the least-recently-used element
		 * and reuse its space for the new element. */
		entry = CacheArray_FindMinLRU(cache->arr, cache->cap);
		// Remove evicted element from the rax.
		raxRemove(cache->lookup, (unsigned  char *)entry->key, strlen(entry->key),
		  	NULL);
		CacheArray_CleanEntry(entry, cache->free_entry);

	} else {
		// The array has not yet been filled, use the next available entry.
		entry = cache->arr + cache->size++;
	}
	char *key = rm_strdup(orig_key);
	// Populate the entry.
	CacheArray_PopulateEntry(cache->counter, entry, key, value);
	atomic_fetch_add(&cache->counter, 1);

	// Add the new entry to the rax.
	raxInsert(cache->lookup, (unsigned char *)key, key_len, entry, NULL);
	pthread_rwlock_unlock(&cache->_cache_rwlock);
	return true;
}

void Cache_Free(Cache *cache) {
	for(size_t i = 0; i < cache->cap; i++) {
		CacheEntry *entry = cache->arr + i;
		if(i < cache->size) {
			rm_free(entry->key);
			cache->free_entry(entry->value);
		}
	}
	rm_free(cache->arr);
	raxFree(cache->lookup);
	assert(pthread_rwlock_destroy(&cache->_cache_rwlock) == 0);
	rm_free(cache);
}

