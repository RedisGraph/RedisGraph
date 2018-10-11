/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "store.h"
#include "store_type.h"

/* Declaration of the type for redis registration. */
RedisModuleType *StoreRedisModuleType;

void* StoreType_RdbLoad(RedisModuleIO *rdb, int encver) {
    /* Format:
     * id 
     * label
     * #properties
     * properties */
    
    LabelStore *s = calloc(1, sizeof(LabelStore));

    s->id = RedisModule_LoadUnsigned(rdb);
    s->label = strdup(RedisModule_LoadStringBuffer(rdb, NULL));
    s->properties = NewTrieMap();

    uint64_t propCount = RedisModule_LoadUnsigned(rdb);
    for(int i = 0; i < propCount; i++) {
        size_t propLen;
        char *prop = RedisModule_LoadStringBuffer(rdb, &propLen);
        TrieMap_Add(s->properties, prop, propLen, NULL, NULL);
    }

    return s;
}

void StoreType_RdbSave(RedisModuleIO *rdb, void *value) {
    /* Format:
     * id 
     * label
     * #properties
     * properties */

    LabelStore *s = value;

    RedisModule_SaveUnsigned(rdb, s->id);
    RedisModule_SaveStringBuffer(rdb, s->label, strlen(s->label) + 1);
    RedisModule_SaveUnsigned(rdb, s->properties->cardinality);

    if(s->properties->cardinality) {
        char *ptr;
        tm_len_t len;
        void *v;
        TrieMapIterator *it = TrieMap_Iterate(s->properties, "", 0);
        while(TrieMapIterator_Next(it, &ptr, &len, &v)) {
            RedisModule_SaveStringBuffer(rdb, ptr, len);
        }
        TrieMapIterator_Free(it);
    }
}

void StoreType_AofRewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    // TODO: implement.
}

void StoreType_Free(void *value) {
    LabelStore *s = value;
    LabelStore_Free(s);
}

