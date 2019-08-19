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
	 * exact-match index exists (boolean)
	 *   exact-match index
	 * fulltext index exists (boolean)
	 *   fulltext index */

	// Schema ID.
	RedisModule_SaveUnsigned(rdb, s->id);

	// Schema name.
	RedisModule_SaveStringBuffer(rdb, s->name, strlen(s->name) + 1);

	// Boolean marking presence of an exact-match index.
	bool exact_index_exists = s->index != NULL;
	RedisModule_SaveUnsigned(rdb, exact_index_exists);

	// Exact match index.
	if(exact_index_exists) RdbSaveIndex(rdb, s->index);

	// Boolean marking presence of a full-text index.
	bool ft_index_exists = s->fulltextIdx != NULL;
	RedisModule_SaveUnsigned(rdb, ft_index_exists);

	// Fulltext index.
	if(ft_index_exists) RdbSaveIndex(rdb, s->fulltextIdx);
}
