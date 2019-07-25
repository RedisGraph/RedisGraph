/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_schema.h"
#include "encode_index.h"
#include "../../../util/arr.h"
#include "../../../util/rmalloc.h"

void RdbSaveSchema(RedisModuleIO *rdb, Schema *s) {
    /* Format:
     * id
     * name
     * #indicies M
     * (indicies) X M */

    // Schema ID.
    RedisModule_SaveUnsigned(rdb, s->id);

    // Schema name.
    RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);

    // Number of indicies.
    RedisModule_SaveUnsigned(rdb, Schema_IndexCount(s));

    // Exact match index.
    if(s->index) RdbSaveIndex(rdb, s->index);
    
    // Fulltext index.
    if(s->fulltextIdx) RdbSaveIndex(rdb, s->fulltextIdx);
}
