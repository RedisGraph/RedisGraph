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
static inline hash_key_t _Cache_HashKey(const char *key, size_t keyLen) {
	return  XXH64(key, keyLen, 0);
}

static inline void _Cache_DataFree(void *voidPtr) {
	CacheData *cacheData = voidPtr;
	if(cacheData->freeFunc) cacheData->freeFunc(cacheData->value);
}

inline Cache *Cache_New(size_t cacheSize, cacheValueFreeFunc freeCB) {
	// Memory allocations.
	Cache *cache = rm_malloc(sizeof(Cache));
	// Members initialization.
	cache->priorityQueue = PriorityQueue_New(cacheSize, CacheData, (QueueDataFreeFunc)_Cache_DataFree);
	cache->rt = raxNew();
	cache->cacheValueFree = freeCB;
	return cache;
}

inline void *Cache_GetValue(Cache *cache, const char *key, size_t keyLen) {
	unsigned long long const hashKey = _Cache_HashKey(key, keyLen);
	CacheData *cacheData = raxFind(cache->rt, (unsigned char *)&hashKey, HASH_KEY_LENGTH);
	if(cacheData != raxNotFound) PriorityQueue_AggressivePromotion(cache->priorityQueue, cacheData);
	return (cacheData && cacheData != raxNotFound) ? cacheData->value : NULL;
}

void Cache_SetValue(Cache *cache, const char *key, size_t keyLen, void *value) {
	CacheData cacheData;
	cacheData.hashKey = _Cache_HashKey(key, keyLen);
	cacheData.value = value;
	cacheData.freeFunc = cache->cacheValueFree;
	// See if no one managed to cache it before.
	assert(raxFind(cache->rt, (unsigned char *)&cacheData.hashKey, HASH_KEY_LENGTH) == raxNotFound);
	// Remove less recently used entry.
	if(PriorityQueue_IsFull(cache->priorityQueue)) {
		// Get cache data from queue.
		CacheData *evictedCacheData = (CacheData *)PriorityQueue_Dequeue(cache->priorityQueue);
		// Remove from storage.
		raxRemove(cache->rt, (unsigned char *)&evictedCacheData->hashKey, HASH_KEY_LENGTH, NULL);
	}
	// Add to PriorityQueue.
	CacheData *insertedCacheData = (CacheData *)PriorityQueue_Enqueue(cache->priorityQueue, &cacheData);
	// Store in storage.
	raxInsert(cache->rt, (unsigned char *)&insertedCacheData->hashKey, HASH_KEY_LENGTH,
			  insertedCacheData, NULL);

}

inline void Cache_Free(Cache *cache) {
	// Members destructors.
	PriorityQueue_Free(cache->priorityQueue);
	raxFree(cache->rt);
	// Memory release.
	rm_free(cache);
}
