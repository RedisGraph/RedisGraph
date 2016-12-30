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

int CacheGroupIterNext(khiter_t* iter, char** key, Group** group) {
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

// Constructs group's key.
char* ConcatKeys(const Vector* keys) {
    int concatKeyLen = 0;
    RedisModuleString* key;

    for(int i = 0; i < Vector_Size(keys); i++) {
        Vector_Get(keys, i, &key);

        size_t keyLen;
        RedisModule_StringPtrLen(key, &keyLen);
        
        concatKeyLen += keyLen + 1; // Account for comma
    }
    
    char* concatKey = malloc(sizeof(char) * (concatKeyLen + 1));
    int offset = 0;
    for(int i = 0; i < Vector_Size(keys); i++) {
        Vector_Get(keys, i, &key);

        size_t keyLen = 0;
        const char* strKey = RedisModule_StringPtrLen(key, &keyLen);
        strcpy(concatKey + offset, strKey);
        offset += keyLen;
        concatKey[offset] = ',';
        offset++;
        concatKey[offset] = NULL;
    }

    // Remove last comma
    concatKey[strlen(concatKey)-1] = NULL;
    return concatKey;
}

void CacheGroupAdd(const Group* group) {
    __initGroupCache();

    // Construct key.    
    char* key = ConcatKeys(group->keys);
    int res = kh_set(khID, __groupCache, key, group);
    free(key);
}

void CacheGroupRemove(const Group* group) {
    printf("In CacheGroupRemove\n");
    if(__groupCache == NULL) {
        return;
    }

    char* key = ConcatKeys(group->keys);

    khiter_t iter = kh_get(khID, __groupCache, key);
    if (iter != kh_end(__groupCache)) {
        kh_del(khID, __groupCache, iter);
    }

    free(key);
}

// Retrives a group,
// Returns NULL if group is missing.
void CacheGroupGet(Vector* keys, Group** group) {
    if(__groupCache == NULL) {
        *group = NULL;
        return;
    }

    char* key = ConcatKeys(keys);
    *group = kh_get_val(khID, __groupCache, key, NULL);

    // TODO: freeing key will cause cache not to find it in future calls to CacheGroupGet
    // LEAK
    // free(key);
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