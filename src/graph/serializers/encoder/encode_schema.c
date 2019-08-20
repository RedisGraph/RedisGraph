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
	 * #index types
	 * (indices) X M */

	// Schema ID.
	RedisModule_SaveUnsigned(rdb, s->id);

	// Schema name.
	RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);

	// We have at most 2 index types, if the schema includes both exact-match and full-text indices.
	uint index_type_count = (s->index != NULL) + (s->fulltextIdx != NULL);
	RedisModule_SaveUnsigned(rdb, index_type_count);

	// Exact match index.
	if(s->index) RdbSaveIndex(rdb, s->index);

	// Fulltext index.
	if(s->fulltextIdx) RdbSaveIndex(rdb, s->fulltextIdx);
}
