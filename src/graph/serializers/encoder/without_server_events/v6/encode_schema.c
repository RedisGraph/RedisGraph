/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v6.h"
#include "../../../../../util/arr.h"
#include "../../../../../util/rmalloc.h"

static inline void _RdbSaveIndexData(RedisModuleIO *rdb, Index *idx) {
	if(!idx) return;

	for(uint i = 0; i < idx->fields_count; i++) {
		// Index type
		RedisModule_SaveUnsigned(rdb, idx->type);
		// Indexed property
		RedisModule_SaveStringBuffer(rdb, idx->fields[i], strlen(idx->fields[i]) + 1);
	}
}

void RdbSaveSchema_v6(RedisModuleIO *rdb, Schema *s) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * (index type, indexed property) X M */

	// Schema ID.
	RedisModule_SaveUnsigned(rdb, s->id);

	// Schema name.
	RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);

	// Number of indices.
	RedisModule_SaveUnsigned(rdb, Schema_IndexCount(s));

	// Exact match indices.
	_RdbSaveIndexData(rdb, s->index);

	// Fulltext indices.
	_RdbSaveIndexData(rdb, s->fulltextIdx);
}
