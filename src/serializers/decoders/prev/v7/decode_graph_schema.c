/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v7.h"

static Schema *_RdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
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
	char *fields[index_count];
	IndexType types[index_count];
	/* There is a compatibility issue in the encoding format v7
	 * in which IndexType will have the values:
	 * [IDX_EXACT_MATCH, IDX_FULLTEXT]
	 * in earlier RDBs, and:
	 * [IDX_ANY, IDX_EXACT_MATCH, IDX_FULLTEXT]
	 * in later ones.
	 * As such, we will iterate over the serialized values looking
	 * for invalid type values of 0 (IDX_ANY). If any are found,
	 * all types will be incremented by 1, causing correct deserialization of
	 * indexes unless an early RDB had only full-text indexes. */
	bool adjust_for_idx_any = false;
	for(uint i = 0; i < index_count; i++) {
		IndexType type = RedisModule_LoadUnsigned(rdb);
		types[i] = type;
		if(type == IDX_ANY) adjust_for_idx_any = true; // Invalid IDX_ANY value found.
		char *field = RedisModule_LoadStringBuffer(rdb, NULL);
		fields[i] = field;
	}

	for(uint i = 0; i < index_count; i++) {
		if(adjust_for_idx_any) types[i] += 1; // Adjust for invalid IDX_ANY value.
		IndexField field = IndexField_New(
				rm_strdup(fields[i]),
				INDEX_FIELD_DEFAULT_WEIGHT,
				INDEX_FIELD_DEFAULT_NOSTEM,
				INDEX_FIELD_DEFAULT_PHONETIC);
		Schema_AddIndex(&idx, s, &field, types[i]);
		RedisModule_Free(fields[i]);
	}

	return s;
}

static void _RdbLoadAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #attribute keys
	 * attribute keys
	 */

	uint count = RedisModule_LoadUnsigned(rdb);
	for(uint i = 0; i < count; i ++) {
		char *attr = RedisModule_LoadStringBuffer(rdb, NULL);
		GraphContext_FindOrAddAttribute(gc, attr);
		RedisModule_Free(attr);
	}
}

void RdbLoadGraphSchema_v7(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * attribute keys (unified schema)
	 * #node schemas
	 * node schema X #node schemas
	 * #relation schemas
	 * unified relation schema
	 * relation schema X #relation schemas
	 */

	// Attributes, Load the full attribute mapping.
	_RdbLoadAttributeKeys(rdb, gc);

	// #Node schemas
	uint schema_count = RedisModule_LoadUnsigned(rdb);

	// Load each node schema
	gc->node_schemas = array_ensure_cap(gc->node_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		array_append(gc->node_schemas, _RdbLoadSchema(rdb, SCHEMA_NODE));
	}

	// #Edge schemas
	schema_count = RedisModule_LoadUnsigned(rdb);

	// Load each edge schema
	gc->relation_schemas = array_ensure_cap(gc->relation_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		array_append(gc->relation_schemas, _RdbLoadSchema(rdb, SCHEMA_EDGE));
	}
}
