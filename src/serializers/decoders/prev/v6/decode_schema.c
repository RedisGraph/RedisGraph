/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v6.h"

Schema *RdbLoadSchema_v6(RedisModuleIO *rdb, SchemaType type) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * (index type, indexed property) X M */

	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = Schema_New(SCHEMA_NODE, id, name);
	RedisModule_Free(name);

	Index *idx = NULL;
	uint index_count = RedisModule_LoadUnsigned(rdb);
	for(uint i = 0; i < index_count; i++) {
		IndexType type = RedisModule_LoadUnsigned(rdb);
		char *field_name = RedisModule_LoadStringBuffer(rdb, NULL);
		IndexField field;
		IndexField_New(&field, field_name, INDEX_FIELD_DEFAULT_WEIGHT,
				INDEX_FIELD_DEFAULT_NOSTEM, INDEX_FIELD_DEFAULT_PHONETIC);
		Schema_AddIndex(&idx, s, &field, type);
		RedisModule_Free(field_name);
	}

	return s;
}

