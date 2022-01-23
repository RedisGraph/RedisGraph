/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stddef.h>
#include "group_cache.h"
#include "../util/rmalloc.h"

CacheGroup *CacheGroupNew() {
	return raxNew();
}

void CacheGroupAdd(CacheGroup *groups, XXH64_hash_t key, Group *group) {
	raxInsert(groups, (unsigned char *)&key, sizeof(key), group, NULL);
}

// retrives a group, sets group to NULL if key is missing
Group *CacheGroupGet(CacheGroup *groups, XXH64_hash_t key) {
	Group *g = raxFind(groups, (unsigned char *)&key, sizeof(key));
	if(g == raxNotFound) return NULL;
	return g;
}

void FreeGroupCache(CacheGroup *groups) {
	raxFreeWithCallback(groups, (void (*)(void *))FreeGroup);
}

// Populates an iterator to scan entire group cache
CacheGroupIterator *CacheGroupIter(CacheGroup *groups) {
	CacheGroupIterator *iter = rm_malloc(sizeof(CacheGroupIterator));

	raxStart(iter, groups);
	raxSeek(iter, "^", NULL, 0);

	return iter;
}

// advance iterator and returns value in current position
int CacheGroupIterNext(CacheGroupIterator *iter, Group **group) {
	int res = raxNext(iter);
	if(res == 0) {
		*group = NULL;
	} else {
		*group = iter->data; // TODO revisit this to fix up
	}
	return res;
}

void CacheGroupIterator_Free(CacheGroupIterator *iter) {
	if(iter == NULL) return;
	raxStop(iter);
	rm_free(iter);
}
