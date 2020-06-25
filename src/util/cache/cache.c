/*
 *Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 *This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache.h"
#include "xxhash.h"
#include "../rmalloc.h"

/**
 * @brief  Hash a key using XXHASH into a 64 bit.
 * @param  *key - string to be hashed.
 * @param  len: key length
 * @retval hash value
 */
static inline uint64_t _Cache_HashKey(const char *key, uint len) {
	return XXH64(key, len, 0);
}

Cache *Cache_New(uint size, CacheItemFreeFunc freeCB) {
	Cache *cache = rm_malloc(sizeof(Cache));
	// Instantiate a new list to store cached values.
	cache->list = CacheList_New(size, freeCB);
	// Instantiate the lookup map for fast cache retrievals.
	cache->lookup = raxNew();
	return cache;
}

inline void *Cache_GetValue(const Cache *cache, const char *key) {
	uint64_t hashKey = _Cache_HashKey(key, strlen(key));
	CacheListNode *elem = raxFind(cache->lookup, (unsigned char *)&hashKey, HASH_KEY_LENGTH);
	if(elem == raxNotFound) return NULL;

	// Element is now the most recently used; promote it.
	CacheList_Promote(cache->list, elem);
	return elem->value;
}

void Cache_SetValue(Cache *cache, const char *key, void *value) {
	uint64_t hashval = _Cache_HashKey(key, strlen(key));
	CacheListNode *node;
	if(CacheList_IsFull(cache->list)) {
		/* The list is full, evict the least-recently-used element
		 * and reuse its space for the new entry. */
		node = CacheList_RemoveTail(cache->list);
		// Remove evicted element from the lookup map.
		raxRemove(cache->lookup, (unsigned char *)&node->hashval, HASH_KEY_LENGTH, NULL);
	} else {
		// The list has not yet been filled, introduce a new node.
		node = CacheList_GetUnused(cache->list);
	}

	// Populate the node.
	CacheList_PopulateNode(cache->list, node, hashval, value);

	// Add the new node to the mapping.
	raxInsert(cache->lookup, (unsigned char *)&hashval, HASH_KEY_LENGTH, node, NULL);
}

void Cache_Free(Cache *cache) {
	CacheList_Free(cache->list);
	raxFree(cache->lookup);
	rm_free(cache);
}

