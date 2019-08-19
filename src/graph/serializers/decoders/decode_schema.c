/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_schema.h"
#include "decode_index.h"

Schema *RdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
	/* Format:
	 * id
	 * name
	 * exact-match index exists (boolean)
	 *   exact-match index
	 * fulltext index exists (boolean)
	 *   fulltext index */

	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = Schema_New(name, id);

	bool exact_index_exists = RedisModule_LoadUnsigned(rdb);
	if(exact_index_exists) s->index = RdbLoadIndex(rdb);

	bool ft_index_exists = RedisModule_LoadUnsigned(rdb);
	if(ft_index_exists) s->fulltextIdx = RdbLoadIndex(rdb);

	return s;
}
