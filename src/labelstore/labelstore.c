#include "labelstore.h"
#include "../rmutil/strings.h"
#include "../redismodule.h"

// TODO: Switch from Redis sorted-set to trie.

// Given a label, returns a key to that label store (Redis sorted-set).
RedisModuleKey* _GetLabelStoreKey(RedisModuleCtx *ctx, const RedisModuleString *graph, const RedisModuleString *label) {
    // Build key
    const char *strGraph = RedisModule_StringPtrLen(graph, NULL);
    const char *strLabel = RedisModule_StringPtrLen(label, NULL);
    RedisModuleString *strKey = RMUtil_CreateFormattedString(ctx, "%s_%s_%s", LABEL_PREFIX, strGraph, strLabel);

    // Get a hold of the key
    RedisModuleKey *key = RedisModule_OpenKey(ctx, strKey, REDISMODULE_WRITE);
    RedisModule_FreeString(ctx, strKey);

    // Make sure key is either empty or sorted set.
    int keytype = RedisModule_KeyType(key);
    if(keytype != REDISMODULE_KEYTYPE_ZSET && keytype != REDISMODULE_KEYTYPE_EMPTY) {
        return NULL;
    }

    return key;
}

int LabelEntity(RedisModuleCtx *ctx, const RedisModuleString *graph, const RedisModuleString *label, const RedisModuleString *entityId) {
    RedisModuleKey *key = _GetLabelStoreKey(ctx, graph, label);
    if(key == NULL) {
        return 0;
    }

    RedisModule_ZsetAdd(key, LABEL_DEFAULT_SCORE, entityId, NULL);
    RedisModule_CloseKey(key);
    return 1;
}

// Returns an iterator to a label store,
// Use this iterator to scan through label store content.
LabelStoreIterator* LabelGetIter(RedisModuleCtx *ctx, const RedisModuleString *graph, const RedisModuleString *label) {
    RedisModuleKey *key = _GetLabelStoreKey(ctx, graph, label);
    if(key == NULL) {
        return NULL;
    }
    
    if (RedisModule_ZsetFirstInScoreRange(key, REDISMODULE_NEGATIVE_INFINITE, REDISMODULE_POSITIVE_INFINITE, 1, 1) != REDISMODULE_OK) {
        return NULL;
    }

    LabelStoreIterator *iter = malloc(sizeof(LabelStoreIterator));
    iter->key = key;
    iter->closed = 0;
    return iter;
}

// Closes given cursor
void _LabelIterClose(LabelStoreIterator *iter) {
	RedisModule_CloseKey(iter->key);
	iter->closed = 1;
}

RedisModuleString* LabelIterNext(LabelStoreIterator *iter) {
    if(iter->closed) {
		return NULL;
	}

	double dScore = 0.0;
	RedisModuleString* elementId =
		RedisModule_ZsetRangeCurrentElement(iter->key, &dScore);
    
    // iterator reached it's end?
    if (!RedisModule_ZsetRangeNext(iter->key)) {
        _LabelIterClose(iter);
	}

	return elementId;
}

void LabelIterFree(LabelStoreIterator *iter) {
    if(iter == NULL) { return; }
    
    // Make sure to close iterator.
    if(iter->closed == 0) {
        _LabelIterClose(iter);
    }
    
    free(iter);
}
