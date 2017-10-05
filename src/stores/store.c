#include "store.h"
#include "../rmutil/strings.h"
#include "../util/triemap/triemap_type.h"

Store *_NewStore() {
	return NewTrieMap();
}

int Store_ID(char **id, StoreType type, const char *graph, const char *label) {
    if(label == NULL) {
        label = "ALL";
    }

    const char* storeType = NULL;

    switch(type) {
        case STORE_NODE:
            storeType = "NODE";
            break;
        case STORE_EDGE:
            storeType = "EDGE";
            break;
        default:
            // Unexpected store type.
            break;
    }
    
    return asprintf(id, "%s_%s_%s_%s", STORE_PREFIX, graph, storeType, label);
}

Store *GetStore(RedisModuleCtx *ctx, StoreType type, const char *graph, const char* label) {
	Store *store = NULL;
    char *strKey;
    Store_ID(&strKey, type, graph, label);

    RedisModuleString *rmStoreId = RedisModule_CreateString(ctx, strKey, strlen(strKey));
    free(strKey);
    
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rmStoreId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, rmStoreId);

	if (RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) {
		store = _NewStore();
		RedisModule_ModuleTypeSetValue(key, TrieRedisModuleType, store);
	}

	store = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);
	return store;
}

int Store_Cardinality(Store *store) {
    return store->cardinality;
}

void Store_Insert(Store *store, char *id, void *value) {
    TrieMap_Add(store, id, strlen(id), value, NULL);
}

int Store_Remove(Store *store, char *id, void (*freeCB)(void *)) {
    return TrieMap_Delete(store, id, strlen(id), freeCB);
}

StoreIterator *Store_Search(Store *store, const char *prefix) {
    char* prefix_dup = strdup(prefix);
	StoreIterator *iter = TrieMap_Iterate(store, prefix_dup, strlen(prefix_dup));
    return iter;
}

void Store_Search_Iter(Store *store, const char *prefix, StoreIterator *it) {
    char* prefix_dup = strdup(prefix);
    TrieMapIterator_Reset(it, store, prefix_dup, strlen(prefix_dup));
}

void *Store_Get(Store *store, char *id) {
    void *val = TrieMap_Find(store, id, strlen(id));
    if(val == TRIEMAP_NOTFOUND) {
        val = NULL;
    }
    return val;
}

void Store_Free(Store *store, void (*freeCB)(void *)) {
    TrieMap_Free(store, freeCB);
}

int StoreIterator_Next(StoreIterator *cursor, char **key, tm_len_t *len, void **value) {
    return TrieMapIterator_Next(cursor, key, len, value);
}

void StoreIterator_Free(StoreIterator* iterator) {
	TrieMapIterator_Free(iterator);
}