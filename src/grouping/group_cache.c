#include "group_cache.h"
#include "../redismodule.h"

static khash_t(khID) *__groupCache = NULL;

static void __initGroupCache() {
    if(__groupCache == NULL) {
        __groupCache = kh_init(khID);
    }
}

khiter_t CacheGroupIter() {
    return kh_begin(__groupCache);
}

int CacheGroupIterNext(khiter_t *iter, char **key, Group **group) {
    for(;*iter != kh_end(__groupCache); ++*iter) {
        if(kh_exist(__groupCache, *iter)) {
            *key = kh_key(__groupCache, *iter);
            *group = kh_value(__groupCache, *iter);
            ++*iter; // Advance before returning.
            return 1;
        }
    }
    return 0;
}

int CacheGroupAdd(const char *key, Group *group) {
    __initGroupCache();
    return kh_set(khID, __groupCache, key, group);
}

void CacheGroupRemove(const char *key) {
    if(__groupCache == NULL) {
        return;
    }

    khiter_t iter = kh_get(khID, __groupCache, key);
    if (iter != kh_end(__groupCache)) {
        kh_del(khID, __groupCache, iter);
    }
}

// Retrives a group,
// Returns NULL if group is missing.
void CacheGroupGet(const char *key, Group **group) {
    if(__groupCache == NULL) {
        *group = NULL;
        return;
    }
    *group = kh_get_val(khID, __groupCache, key, NULL);
}

void CacheGroupClear() {
    if(__groupCache == NULL) {
        return;
    }

    for (khiter_t iter = kh_begin(__groupCache); iter != kh_end(__groupCache); ++iter) {
        if (kh_exist(__groupCache, iter)) {
            kh_del(khID, __groupCache, iter);
        }
    }

    // Cleanup and remove our hashtable
    // kh_destroy(khID, __groupCache);
}