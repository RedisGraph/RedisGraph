/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

//------------------------------------------------------------------------------
// function pointers
//------------------------------------------------------------------------------

// cache entry free function
typedef void (*CacheEntryFreeFunc)(void *);

// cache entry duplicate function
typedef void *(*CacheEntryCopyFunc)(void *);

 // cache entry
typedef struct CacheEntry_t {
	char *key;      // entry key
	void *value;    // entry value
	long long LRU;  // indicates the time when the entry was last used
} CacheEntry;


// returns a pointer to the entry in the cache array with the minimum LRU
CacheEntry *CacheArray_FindMinLRU
(
	CacheEntry *cache_arr,  // list of cached items
	uint cap                // number of elements in list
);

// assign new values to the fields of a cache entry
void CacheArray_PopulateEntry
(
	CacheEntry *entry,  // entry to set
	long long counter,  // time of update
	char *key,          // key associated with entry
	void *value         // value associated with entry
);

// free the fields of a cache entry to prepare it for reuse
void CacheArray_ClearEntry
(
	CacheEntry *entry,             // entry to clear
	CacheEntryFreeFunc free_entry  // free callback
);

