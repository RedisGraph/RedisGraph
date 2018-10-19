/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "group_cache.h"
#include "../util/vector.h"

void CacheGroupAdd(TrieMap *groups, char *key, Group *group) {
    TrieMap_Add(groups, key, strlen(key), group, NULL);
}

// Retrives a group,
// Sets group to NULL if key is missing.
void CacheGroupGet(TrieMap *groups, char *key, Group **group) {
    *group = TrieMap_Find(groups, key, strlen(key));
    if (*group == TRIEMAP_NOTFOUND) {
        *group = NULL;
    }
}

void FreeGroupCache(TrieMap *groups) {
    TrieMap_Free(groups, (void (*)(void *))FreeGroup);
}

// Returns an iterator to scan entire group cache
CacheGroupIterator* CacheGroupIter(TrieMap *groups) {
    char *prefix = "";
	return TrieMap_Iterate(groups, prefix, strlen(prefix));
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
