/*
 * Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "cache_array.h"
#include "rax.h"

 // Key-value cache, uses LRU policy for eviction
 // assumes owership over stored objects
typedef struct Cache {
	uint cap;                          // cache capacity
	uint size;                         // cache current size
	long long counter;                 // atomic counter for number of reads
	rax *lookup;                       // mapping between keys to entries
	CacheEntry *arr;                   // array of cache elements
	CacheEntryFreeFunc free_item;      // callback that free cached value
	CacheEntryCopyFunc copy_item;      // callback that copies cached value
	pthread_rwlock_t _cache_rwlock;    // read-write lock to protect access
} Cache;

// initialize a cache, returns an empty cache
Cache *Cache_New
(
	uint cap,                     // number of entries
	CacheEntryFreeFunc freeFunc,  // callback for freeing the stored values
	CacheEntryCopyFunc copyFunc   // callback for copying cached items
);

// clear all entries from cache
void Cache_Clear
(
	Cache *cache  // cache to clear
);

// returns a copy of value if it is cached, NULL otherwise 
void *Cache_GetValue
(
	Cache *cache,    // cache to retrieve value from
	const char *key  // key to look for
);

// stores value under key within the cache
// in case the cache is full, this operation causes a cache eviction
void Cache_SetValue
(
	Cache *cache,     // cache to populate
	const char *key,  // key associated with value
	void *value       // value to store
);

// stores value under key within the cache, and return a copy of that value
// in case the cache is full, this operation causes a cache eviction
// returns a copy of the given value if a new item was added to the cache
// the original value if it was already exist
void *Cache_SetGetValue
(
	Cache *cache,     // cache pointer
	const char *key,  // key for associating with value
	void *value       // pointer with the relevant value
);

// destroys the cache and free all stored items
void Cache_Free
(
	Cache *cache // cache pointer
);

