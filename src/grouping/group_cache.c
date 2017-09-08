#include "group_cache.h"
#include "../rmutil/vector.h"

void InitGroupCache() {
    __groupCache = NewTrieMap();
}

void CacheGroupAdd(char *key, Group *group) {
    TrieMap_Add(__groupCache, key, strlen(key), group, NULL);
}

// Retrives a group,
// Sets group to NULL if key is missing.
void CacheGroupGet(char *key, Group **group) {
    *group = TrieMap_Find(__groupCache, key, strlen(key));
    if (*group == TRIEMAP_NOTFOUND) {
        *group = NULL;
    }
}

void FreeGroupCache() {
    TrieMap_Free(__groupCache, (void (*)(void *))FreeGroup);
}

// Returns an iterator to scan entire group cache
CacheGroupIterator* CacheGroupIter() {
    char* prefix = strdup("");
	return TrieMap_Iterate(__groupCache, prefix, strlen(prefix));
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