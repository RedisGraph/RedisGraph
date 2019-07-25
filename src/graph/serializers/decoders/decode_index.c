/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_index.h"

Index* RdbLoadIndex(RedisModuleIO *rdb) {
    /* Format:
     * type
     * name
     * #fields M
     * (fields) X M */

    IndexType type = RedisModule_LoadUnsigned(rdb);
    char *name = RedisModule_LoadStringBuffer(rdb, NULL);
    Index *idx = Index_New(name, type);
    
    uint field_count = RedisModule_LoadUnsigned(rdb);
    for(uint i = 0; i < field_count; i++) {
        char *field = RedisModule_LoadStringBuffer(rdb, NULL);
        Index_AddField(idx, field);
    }

    return idx;
}
