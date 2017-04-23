#include "store.h"
#include "../rmutil/strings.h"
#include "../util/triemap/triemap_type.h"

Store *_NewStore() {
	return NewTrieMap();
}

RedisModuleString *_StoreID(RedisModuleCtx *ctx, StoreType type, const RedisModuleString *graph, const RedisModuleString *label) {
    const char *strGraph = RedisModule_StringPtrLen(graph, NULL);
    const char *strLabel = "ALL";
    
    if(label != NULL) {
        strLabel = RedisModule_StringPtrLen(label, NULL);
    }

    const char* strStoreType = NULL;

    switch(type) {
        case STORE_NODE:
            strStoreType = "NODE";
            break;
        case STORE_EDGE:
            strStoreType = "EDGE";
            break;
        default:
            // Unexpected store type.
            break;
    }
    
    RedisModuleString *strKey = RMUtil_CreateFormattedString(ctx, "%s_%s_%s_%s", STORE_PREFIX, strGraph, strStoreType, strLabel);
    return strKey;
}

Store *GetStore(RedisModuleCtx *ctx, StoreType type, const RedisModuleString *graph, const RedisModuleString* label) {
	Store *store = NULL;
    RedisModuleString *strKey = _StoreID(ctx, type, graph, label);
	RedisModuleKey *key =
        RedisModule_OpenKey(ctx, strKey, REDISMODULE_WRITE);

	int keyType = RedisModule_KeyType(key);

	if (keyType == REDISMODULE_KEYTYPE_EMPTY) {
		store = _NewStore();
		RedisModule_ModuleTypeSetValue(key, TrieRedisModuleType, store);
	}

	store = RedisModule_ModuleTypeGetValue(key);
	return store;
}

void Store_Insert(Store *store, const RedisModuleString *id) {
    size_t len;
    const char *strId = RedisModule_StringPtrLen(id, &len);
    
    TrieMap_Add(store, id, len, NULL, NULL);
}

void Store_Remove(Store *store, const RedisModuleString *id) {
    size_t len;
    const char *strId = RedisModule_StringPtrLen(id, &len);
    
    TrieMap_Delete(store, strId, len, NULL);
}

StoreIterator *Store_Search(Store *store, const char *prefix) {
    char* prefix_dup = strdup(prefix);
	StoreIterator *iter = TrieMap_Iterate(store, prefix_dup, strlen(prefix_dup));
    return iter;
}

char *StoreIterator_Next(StoreIterator *cursor) {
    char *key = NULL;
	tm_len_t len = 0;
	void *ptr = NULL;
	int res = TrieMapIterator_Next(cursor, &key, &len, &ptr);
	if(res == 0) {
		return NULL;
	}

	return key;
}

void StoreIterator_Free(StoreIterator* iterator) {
	TrieMapIterator_Free(iterator);
}