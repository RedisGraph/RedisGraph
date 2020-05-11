/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v4.h"

Index *RdbLoadIndex_v4(RedisModuleIO *rdb, GraphContext *gc) {
	Index *idx = NULL;
	char *label = RedisModule_LoadStringBuffer(rdb, NULL);
	char *field = RedisModule_LoadStringBuffer(rdb, NULL);

	Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
	Schema_AddIndex(&idx, s, field, IDX_EXACT_MATCH);

	RedisModule_Free(label);
	RedisModule_Free(field);

	return idx;
}
