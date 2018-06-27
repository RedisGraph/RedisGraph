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

int StoreType_Register(RedisModuleCtx *ctx) {
    RedisModuleTypeMethods tm = {.version = REDISMODULE_TYPE_METHOD_VERSION,
                                 .rdb_load = StoreType_RdbLoad,
                                 .rdb_save = StoreType_RdbSave,
                                 .aof_rewrite = StoreType_AofRewrite,
                                 .free = StoreType_Free};
  
    StoreRedisModuleType = RedisModule_CreateDataType(ctx, "storetype", STORE_TYPE_ENCODING_VERSION, &tm);
    if (StoreRedisModuleType == NULL) {
        return REDISMODULE_ERR;
    }
    return REDISMODULE_OK;
}
