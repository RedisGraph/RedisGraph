/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_index.h"
#include "../../../index/index.h"

void RdbSaveIndex(RedisModuleIO *rdb, void *value) {
    /* Format:
     * type
     * name
     * #fields M
     * (fields) X M */
    Index *idx = (Index*)value;
    
    // Index type
    RedisModule_SaveUnsigned(rdb, idx->type);

    // Index name
    RedisModule_SaveStringBuffer(rdb, idx->label, strlen(idx->label) + 1);

    // Index field count
    RedisModule_SaveUnsigned(rdb, idx->fields_count);

    for(uint i = 0; i < idx->fields_count; i++) {
        RedisModule_SaveStringBuffer(rdb, idx->fields[i], strlen(idx->fields[i]) + 1);
    }
}
