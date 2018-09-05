/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

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

void LabelStore_AssignValue(LabelStore *store, char *property, size_t *id) {
    TrieMap_Add(store->properties, property, strlen(property), (void *)id, NULL);
}

void LabelStore_UpdateSchema(LabelStore *store, int prop_count, char **properties) {
    for(int idx = 0; idx < prop_count; idx++) {
        char *property = properties[idx];
        TrieMap_Add(store->properties, property, strlen(property), NULL, TrieMap_NOP_REPLACE);
    }
}

RedisModuleString **LabelStore_GetKeys(RedisModuleCtx *ctx, const char *graphID, size_t *keyCount) {
    assert(ctx && graphID && keyCount);
    char *pattern;
    RedisModuleCallReply *keysCallReply;

    // Search for keys with prefix: 'LABELSTORE_PREFIX_graphID_*'.
    asprintf(&pattern, "%s_%s_*", LABELSTORE_PREFIX, graphID);
    keysCallReply = RedisModule_Call(ctx, "KEYS", "c", pattern);    
    free(pattern);

    if(!keysCallReply) {
        *keyCount = 0;
        NULL;
    }
    
    size_t matchedKeyCount = RedisModule_CallReplyLength(keysCallReply);
    RedisModuleString **storeKeys = malloc(sizeof(RedisModuleString*) * matchedKeyCount);

    int storeKeyIdx = 0;
    for(int idx = 0; idx < matchedKeyCount; idx++) {
        RedisModuleCallReply *reply = RedisModule_CallReplyArrayElement(keysCallReply, idx);
        RedisModuleString *storeKeyStr = RedisModule_CreateStringFromCallReply(reply);
        RedisModuleKey *key = RedisModule_OpenKey(ctx, storeKeyStr, REDISMODULE_WRITE);

        if (RedisModule_ModuleTypeGetType(key) == StoreRedisModuleType) {
            storeKeys[storeKeyIdx++] = storeKeyStr;
        } else {
            RedisModule_Free(storeKeyStr);
        }
        RedisModule_CloseKey(key);
    }

    RedisModule_Free(keysCallReply);
    *keyCount = storeKeyIdx;
    return storeKeys;
}

void LabelStore_Free(LabelStore *store) {
    TrieMap_Free(store->properties, TrieMap_NOP_CB);
    if(store->label) free(store->label);
    free(store);
}
