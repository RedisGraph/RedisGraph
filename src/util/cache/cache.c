/*
 *Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 *This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache.h"
#include "../rmalloc.h"

//------------------------------------------------------------------------------
// dict type callbacks
//------------------------------------------------------------------------------

// dict key compare
int dictStringKeyCompare(void *privdata, const void *key1, const void *key2) {
    return strcmp(key1, key2) == 0;
}

// dict key hash function
uint64_t dictStringHash(const void *key) {
    return dictGenHashFunction(key, strlen(key));
}

// dict type
dictType dictTypeHeapStrings = {
    dictStringHash,             // hash function
    NULL,                       // key dup
    NULL,                       // val dup
    dictStringKeyCompare,       // key compare
    NULL,                       // key destructor
    NULL                        // val destructor
};

Cache *Cache_New(uint size, CacheItemFreeFunc freeCB) {
	Cache *cache = rm_malloc(sizeof(Cache));
	// Instantiate a new list to store cached values.
	cache->list = CacheList_New(size, freeCB);
	// Instantiate the lookup map for fast cache retrievals.
	cache->lookup = dictCreate(&dictTypeHeapStrings, NULL);
	return cache;
}

inline void *Cache_GetValue(const Cache *cache, const char *key) {
	CacheListNode *elem = dictFetchValue(cache->lookup, key);
	if(elem == NULL) return NULL;

	// Element is now the most recently used; promote it.
	CacheList_Promote(cache->list, elem);
	return elem->value;
}

void Cache_SetValue(Cache *cache, const char *key, void *value) {
	CacheListNode *node;
	if(CacheList_IsFull(cache->list)) {
		/* The list is full, evict the least-recently-used element
		 * and reuse its space for the new entry. */
		node = CacheList_RemoveTail(cache->list);
		// Remove evicted element from the lookup map.
		dictDelete(cache->lookup, node->key);
	} else {
		// The list has not yet been filled, introduce a new node.
		node = CacheList_GetUnused(cache->list);
	}

	// Populate the node.
	CacheList_PopulateNode(cache->list, node, key, value);

	// Add the new node to the mapping.
	dictAdd(cache->lookup, (void*)key, node);
}

void Cache_Free(Cache *cache) {
	CacheList_Free(cache->list);
	dictRelease(cache->lookup);
	rm_free(cache);
}

