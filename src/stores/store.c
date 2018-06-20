#include "store.h"
#include "store_type.h"
#include "../rmutil/util.h"
#include "../rmutil/strings.h"
#include <assert.h>

/* Generates an ID for a new LabelStore. */
int LabelStore_Id(char **id, LabelStoreType type, const char *graph, const char *label) {
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
    
    return asprintf(id, "%s_%s_%s_%s", LABELSTORE_PREFIX, graph, storeType, label);
}

/* Creates a new LabelStore. */
LabelStore *LabelStore_New(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label, int id) {
    assert(ctx && graph && label);

    LabelStore *store = calloc(1, sizeof(LabelStore));
    store->properties = NewTrieMap();
    store->label = strdup(label);
    store->id = id;
    
    // Store labelstore within redis keyspace.
    char *strKey;
    LabelStore_Id(&strKey, type, graph, label);

    RedisModuleString *rmStoreId = RedisModule_CreateString(ctx, strKey, strlen(strKey));
    free(strKey);
    
    RedisModuleKey *key = RedisModule_OpenKey(ctx, rmStoreId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, rmStoreId);
    assert(RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY);
    
    RedisModule_ModuleTypeSetValue(key, StoreRedisModuleType, store);
        
    return store;
}

void LabelStore_Free(LabelStore *store) {
    TrieMap_Free(store->properties, TrieMap_NOP_CB);
    if(store->label) free(store->label);
    free(store);
}

LabelStore *LabelStore_Get(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, const char* label) {
	LabelStore *store = NULL;
    char *strKey;
    LabelStore_Id(&strKey, type, graph, label);

    RedisModuleString *rmStoreId = RedisModule_CreateString(ctx, strKey, strlen(strKey));
    free(strKey);
    
	RedisModuleKey *key = RedisModule_OpenKey(ctx, rmStoreId, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, rmStoreId);

    /* Create "ALL" store if doesn't exists. */
    if((RedisModule_KeyType(key) == REDISMODULE_KEYTYPE_EMPTY) && label == NULL) {
        return LabelStore_New(ctx, type, graph, "ALL", -1);
    }

    if (RedisModule_ModuleTypeGetType(key) != StoreRedisModuleType) {
        return NULL;
	}

	store = RedisModule_ModuleTypeGetValue(key);
    RedisModule_CloseKey(key);
	return store;
}

/* Get all stores of given type. */
void LabelStore_Get_ALL(RedisModuleCtx *ctx, LabelStoreType type, const char *graph, LabelStore **stores, size_t *stores_len) {
    char *pattern;
    LabelStore_Id(&pattern, type, graph, "*");

    size_t key_count = 128;             /* Maximum number of keys we're willing to process. */
    RedisModuleString *str_keys[128];   /* Keys returned by SCAN. */

    RMUtil_SCAN(ctx, pattern, str_keys, &key_count);
    free(pattern);

    /* Consume SCAN */
    for(int i = 0; i < key_count; i++) {
        RedisModuleString *store_key = str_keys[i];
        if(i < *stores_len) {
            RedisModuleKey *key = RedisModule_OpenKey(ctx, store_key, REDISMODULE_WRITE);
            stores[i] = RedisModule_ModuleTypeGetValue(key);
            RedisModule_CloseKey(key);
        }
        RedisModule_FreeString(ctx, store_key);
    }

    /* Update number of stores fetched. */
    *stores_len = key_count;
}

void LabelStore_UpdateSchema(LabelStore *store, int prop_count, char **properties) {
    for(int idx = 0; idx < prop_count; idx++) {
        char *property = properties[idx];
        TrieMap_Add(store->properties, property, strlen(property), NULL, NULL);
    }
}