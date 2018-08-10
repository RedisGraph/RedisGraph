/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef GROUP_CACHE_H_
#define GROUP_CACHE_H_

#include "group.h"
#include "../util/triemap/triemap.h"
#include "../rmutil/vector.h"

typedef TrieMapIterator CacheGroupIterator;
void InitGroupCache();

void CacheGroupAdd(TrieMap *groups, char *key, Group *group);

// Retrives a group,
// Sets group to NULL if key is missing.
void CacheGroupGet(TrieMap *groups, char *key, Group **group);

void FreeGroupCache(TrieMap *groups);

// Returns an iterator to scan hashtable
CacheGroupIterator* CacheGroupIter(TrieMap *groups);
// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *iter, char **key, Group **group);

#endif