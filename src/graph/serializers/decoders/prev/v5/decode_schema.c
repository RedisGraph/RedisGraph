/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v5.h"

Schema *RdbLoadSchema_v5(RedisModuleIO *rdb, SchemaType type) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * (index type, indexed property) X M */

	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = Schema_New(name, id);

	Index *idx = NULL;
	uint index_count = RedisModule_LoadUnsigned(rdb);
	for(uint i = 0; i < index_count; i++) {
		IndexType type = RedisModule_LoadUnsigned(rdb);
		char *field = RedisModule_LoadStringBuffer(rdb, NULL);

		Schema_AddIndex(&idx, s, field, type);
	}

	return s;
}
