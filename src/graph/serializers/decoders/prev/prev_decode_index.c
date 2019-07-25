/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "prev_decode_index.h"

Index* PrevRdbLoadIndex(RedisModuleIO *rdb, GraphContext *gc) {
    char *label = RedisModule_LoadStringBuffer(rdb, NULL);
    char *property = RedisModule_LoadStringBuffer(rdb, NULL);

    Index *idx = Index_New(label, IDX_EXACT_MATCH);
    Index_AddField(idx, property);

    return idx;
}
