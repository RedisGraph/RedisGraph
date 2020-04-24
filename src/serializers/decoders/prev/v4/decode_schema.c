/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v4.h"

Schema *RdbLoadSchema_v4(RedisModuleIO *rdb) {
	/* Format:
	 * id
	 * name
	 * #attributes
	 * attributes
	 */

	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = Schema_New(name, id);
	rm_free(name);

	uint64_t attrCount = RedisModule_LoadUnsigned(rdb);

	// Only doing this for backwards compatibility; we're not keeping these strings
	for(uint64_t i = 0; i < attrCount; i++) {
		// Load attribute string from RDB file.
		char *attr = RedisModule_LoadStringBuffer(rdb, NULL);
		RedisModule_Free(attr);
	}

	return s;
}
