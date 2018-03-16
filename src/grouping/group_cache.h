#ifndef GROUP_CACHE_H_
#define GROUP_CACHE_H_

#include "group.h"
#include "../util/triemap/triemap.h"
#include "../rmutil/vector.h"

typedef TrieMap CacheGroup;
typedef TrieMapIterator CacheGroupIterator;

static CacheGroup *__groupCache = NULL;

void InitGroupCache();

void CacheGroupAdd(char *key, Group *group);

// Retrives a group,
// Sets group to NULL if key is missing.
void CacheGroupGet(char *key, Group **group);

void FreeGroupCache();

// Returns an iterator to scan hashtable
CacheGroupIterator* CacheGroupIter();
// Advance iterator and returns key & value in current position.
int CacheGroupIterNext(CacheGroupIterator *iter, char **key, Group **group);

#endif