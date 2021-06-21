/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef GROUP_CACHE_H_
#define GROUP_CACHE_H_

#include "group.h"
#include "rax.h"

typedef raxIterator CacheGroupIterator;
typedef rax CacheGroup;

CacheGroup *CacheGroupNew();

void CacheGroupAdd(CacheGroup *groups, char *key, Group *group);

// adds unsigned long long item as string representation
void CacheGroupAddUll(CacheGroup *groups, unsigned long long key, Group *group);

// Retrives a group,
// Sets group to NULL if key is missing.
Group *CacheGroupGet(CacheGroup *groups, char *key);

// Retrives a group,
// Sets group to NULL if key is missing.
Group *CacheGroupGetUll(CacheGroup *groups, unsigned long long key);

void FreeGroupCache(CacheGroup *groups);

// Populates an iterator to scan group cache
CacheGroupIterator *CacheGroupIter(CacheGroup *groups);

// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *iter, char **key, Group **group);

void CacheGroupIterator_Free(CacheGroupIterator *iter);

#endif