/*
 *Copyright 2018 - 2020 Redis Labs Ltd. and Contributors
 *
 *This file is available under the Redis Labs Source Available License Agreement
 */

#include "cache.h"
#include "xxhash.h"

/**
 * @brief  Hash a query using XXHASH into a 64 bit.
 * @param  *key - string to be hashed.
 * @param  keyLen: key length
 * @retval hash value
 */
static inline hash_key_t _Cache_HashKey(const char *key, uint keyLen) {
	return XXH64(key, keyLen, 0);
}

static inline void _Cache_DataFree(void *voidPtr) {
	CacheData *cacheData = voidPtr;
	if(cacheData->freeFunc) cacheData->freeFunc(cacheData->value);
}

Cache *Cache_New(uint cacheSize, cacheValueFreeFunc freeCB) {
	// Memory allocations.
	Cache *cache = rm_malloc(sizeof(Cache));
	// Members initialization.
	cache->priorityQueue = PriorityQueue_Create(cacheSize, sizeof(CacheData),
												(QueueDataFreeFunc)_Cache_DataFree);
	cache->lookup = raxNew();
	cache->cacheValueFree = freeCB;
	return cache;
}

inline void *Cache_GetValue(Cache *cache, const char *key) {
	hash_key_t hashKey = _Cache_HashKey(key, strlen(key));
	CacheData *cacheData = raxFind(cache->lookup, (unsigned char *)&hashKey, HASH_KEY_LENGTH);
	if(cacheData == raxNotFound) return NULL;
	assert(cacheData); // TODO tmp, delete once guaranteed non-null
	PriorityQueue_AggressivePromotion(cache->priorityQueue, cacheData);
	return cacheData->value;
}

void Cache_SetValue(Cache *cache, const char *key, void *value) {
	CacheData cacheData = {
		.hashKey = _Cache_HashKey(key, strlen(key)),
		.value = value,
		.freeFunc = cache->cacheValueFree,
	};
	// The value should not already be present in the cache.
	assert(raxFind(cache->lookup, (unsigned char *)&cacheData.hashKey, HASH_KEY_LENGTH) == raxNotFound);
	// Remove least recently used entry.
	if(PriorityQueue_IsFull(cache->priorityQueue)) {
		// Get cache data from queue.
		CacheData *evictedCacheData = PriorityQueue_Dequeue(cache->priorityQueue);
		// Remove from storage.
		raxRemove(cache->lookup, (unsigned char *)&evictedCacheData->hashKey, HASH_KEY_LENGTH, NULL);
	}
	// Add to PriorityQueue.
	CacheData *insertedCacheData = (CacheData *)PriorityQueue_Enqueue(cache->priorityQueue, &cacheData);
	// Store in storage.
	raxInsert(cache->lookup, (unsigned char *)&insertedCacheData->hashKey, HASH_KEY_LENGTH,
			  insertedCacheData, NULL);
}

void Cache_Free(Cache *cache) {
	// Members destructors.
	PriorityQueue_Free(cache->priorityQueue);
	raxFree(cache->lookup);
	// Memory release.
	rm_free(cache);
}

