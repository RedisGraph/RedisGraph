/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GROUP_CACHE_H_
#define GROUP_CACHE_H_

#include "group.h"
#include "../util/triemap/triemap.h"

typedef TrieMapIterator CacheGroupIterator;
typedef TrieMap CacheGroup;

CacheGroup* CacheGroupNew();

void CacheGroupAdd(CacheGroup *groups, char *key, Group *group);

// Retrives a group,
// Sets group to NULL if key is missing.
Group* CacheGroupGet(CacheGroup *groups, char *key);

void FreeGroupCache(CacheGroup *groups);

// Returns an iterator to scan hashtable
CacheGroupIterator* CacheGroupIter(CacheGroup *groups);

// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *iter, char **key, Group **group);

void CacheGroupIterator_Free(CacheGroupIterator* iter);

#endif