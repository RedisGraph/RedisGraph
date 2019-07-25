/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_schema.h"
#include "decode_index.h"

Schema* RdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
    /* Format:
     * id
     * name
     * #indicies M
     * (indicies) X M */

    int id = RedisModule_LoadUnsigned(rdb);
    char *name = RedisModule_LoadStringBuffer(rdb, NULL);
    Schema *s = Schema_New(name, id);

    uint index_count = RedisModule_LoadUnsigned(rdb);
    for (uint i = 0; i < index_count; i++) {
        Index *idx = RdbLoadIndex(rdb);
        if(idx->type == IDX_EXACT_MATCH) s->index = idx;
        else s->fulltextIdx = idx;
    }

    return s;
}
