/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "group_cache.h"

CacheGroup* CacheGroupNew() {
    return NewTrieMap();
}

void CacheGroupAdd(CacheGroup *groups, char *key, Group *group) {
    TrieMap_Add(groups, key, strlen(key), group, NULL);
}

// Retrives a group,
// Sets group to NULL if key is missing.
Group* CacheGroupGet(CacheGroup *groups, char *key) {
    Group *g = TrieMap_Find(groups, key, strlen(key));
    if (g == TRIEMAP_NOTFOUND) return NULL;
    return g;
}

void FreeGroupCache(CacheGroup *groups) {
    // TODO figure this out?
    // TrieMap_Free(groups, (void (*)(void *))FreeGroup);
}

// Returns an iterator to scan entire group cache
CacheGroupIterator* CacheGroupIter(CacheGroup *groups) {
    return TrieMap_Iterate(groups, "", 0);
}

// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *iter, char **key, Group **group) {
    tm_len_t len = 0;
    int res = TrieMapIterator_Next(iter, key, &len, (void**)group);
    if(res == 0) {
        *group = NULL;
    }
    return res;
}

void CacheGroupIterator_Free(CacheGroupIterator* iter) {
    if(iter) TrieMapIterator_Free(iter);
}
