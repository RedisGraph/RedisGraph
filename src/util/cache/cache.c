/*
 * Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache.h"
#include "RG.h"
#include "../rmalloc.h"
#include "cache_array.h"
#include <pthread.h>

static CacheEntry *_CacheEvictLRU
(
	Cache *cache
) {
	CacheEntry *entry = CacheArray_FindMinLRU(cache->arr, cache->cap);

	// remove evicted element from the rax
	raxRemove(cache->lookup, (unsigned  char *)entry->key, strlen(entry->key),
			NULL);

	CacheArray_ClearEntry(entry, cache->free_item);

	return entry;
}

static bool _Cache_SetValue
(
	Cache *cache,
	const char *key,
	void *value,
	size_t key_len
) {
	ASSERT(key   != NULL);
	ASSERT(cache != NULL);

	// in case that another working thread had already inserted the item to the
	// cache, no need to re-insert it
	CacheEntry *entry = raxFind(cache->lookup, (unsigned char *)key, key_len);
	if(entry != raxNotFound) {
		return false;
	}

	// key is not in cache! test to see if cache is full?
	if(cache->size == cache->cap) {
		// the cache is full, evict the least-recently-used element
		// and reuse its space for the new element
		entry = _CacheEvictLRU(cache);
	} else {
		// the array has space left in it, use the next available entry
		entry = cache->arr + cache->size++;
	}

	// populate the entry
	char *k = rm_strdup(key);
	cache->counter++;
	CacheArray_PopulateEntry(entry, cache->counter, k, value);

	// add the new entry to the rax
	raxInsert(cache->lookup, (unsigned char *)key, key_len, entry, NULL);

	return true;
}

// initialize a cache, returns an empty cache
Cache *Cache_New
(
	uint cap,                     // number of entries
	CacheEntryFreeFunc freeFunc,  // callback for freeing the stored values
	CacheEntryCopyFunc copyFunc   // callback for copying cached items
) {
	ASSERT(cap > 0);
	ASSERT(copyFunc != NULL);

	Cache *cache = rm_malloc(sizeof(Cache));

	cache->cap       = cap;
	cache->arr       = rm_calloc(cap, sizeof(CacheEntry));  // cached values
	cache->size      = 0;
	cache->lookup    = raxNew();  // instantiate key entry mapping
	cache->counter   = 0;         // initialize counter to zero
	cache->copy_item = copyFunc;
	cache->free_item = freeFunc;

	// initialize the read-write lock to protect access to the cache
	int res = pthread_rwlock_init(&cache->_cache_rwlock, NULL);
	UNUSED(res);
	ASSERT(res == 0);

	return cache;
}

// clear all entries from cache
void Cache_Clear
(
	Cache *cache  // cache to clear
) {
	ASSERT(cache != NULL);

	// quick return if chache is empty
	if(cache->size == 0) {
		return;
	}

	// acquire WRITE lock
	int res = pthread_rwlock_wrlock(&cache->_cache_rwlock);
	UNUSED(res);
	ASSERT(res == 0);

	// recheck cache item count
	// it might be that another thread beat us to clearing the cache
	if(cache->size != 0) {
		// clear lookup
		raxFree(cache->lookup);
		cache->lookup = raxNew();

		// free all cache entries
		for(size_t i = 0; i < cache->size; i++) {
			CacheEntry *entry = cache->arr + i;
			CacheArray_ClearEntry(entry, cache->free_item);
		}

		// reset size and counter
		cache->size    = 0;
		cache->counter = 0;
	}

	// unlock
	res = pthread_rwlock_unlock(&cache->_cache_rwlock);
	ASSERT(res == 0);

	// log cache been cleared
	if(RedisModule_Log != NULL) {
		RedisModule_Log(NULL, REDISMODULE_LOGLEVEL_NOTICE,
				"query cache cleared");
	}
}

// returns a copy of value if it is cached, NULL otherwise
void *Cache_GetValue
(
	Cache *cache,    // cache to retrieve value from
	const char *key  // key to look for
) {
	void *item = NULL;

	ASSERT(key   != NULL);
	ASSERT(cache != NULL);

	size_t key_len = strlen(key);

	int res = pthread_rwlock_rdlock(&cache->_cache_rwlock);
	UNUSED(res);
	ASSERT(res == 0);

	CacheEntry *entry = raxFind(cache->lookup, (unsigned char *)key, key_len);
	if(entry == raxNotFound) {
		goto cleanup;
	}

	// element is now the most recently used; update its LRU
	// note that multiple threads can be here simultaneously
	cache->counter++;
	entry->LRU = cache->counter;

	// return a copy of element
	item = cache->copy_item(entry->value);

cleanup:
	res = pthread_rwlock_unlock(&cache->_cache_rwlock);
	ASSERT(res == 0);
	return item;
}

// stores value under key within the cache
// in case the cache is full, this operation causes a cache eviction
void Cache_SetValue
(
	Cache *cache,     // cache to populate
	const char *key,  // key associated with value
	void *value       // value to store
) {
	ASSERT(key   != NULL);
	ASSERT(cache != NULL);

	size_t key_len = strlen(key);

	// acquire WRITE lock
	int res = pthread_rwlock_wrlock(&cache->_cache_rwlock);
	UNUSED(res);
	ASSERT(res == 0);

	// insert the value to the cache
	_Cache_SetValue(cache, key, value, key_len);

	res = pthread_rwlock_unlock(&cache->_cache_rwlock);
	ASSERT(res == 0);
}

// stores value under key within the cache, and return a copy of that value
// in case the cache is full, this operation causes a cache eviction
// returns a copy of the given value if a new item was added to the cache
// the original value if it was already exist
void *Cache_SetGetValue
(
	Cache *cache,     // cache pointer
	const char *key,  // key for associating with value
	void *value       // pointer with the relevant value
) {
	ASSERT(key   != NULL);
	ASSERT(cache != NULL);

	size_t key_len = strlen(key);
	void *value_to_return = value;

	// acquire WRITE lock
	int res = pthread_rwlock_wrlock(&cache->_cache_rwlock);
	UNUSED(res);
	ASSERT(res == 0);

	// return true if value was added, false if value already in cache
	if(_Cache_SetValue(cache, key, value, key_len)) {
		// return a copy of original value
		value_to_return = cache->copy_item(value);
	}

	res = pthread_rwlock_unlock(&cache->_cache_rwlock);
	ASSERT(res == 0);

	return value_to_return;
}

// destroys the cache and free all stored items
void Cache_Free
(
	Cache *cache // cache pointer
) {
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
	UNUSED(res);
	ASSERT(res == 0);

	rm_free(cache);
}

