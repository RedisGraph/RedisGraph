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
	Schema *s = Schema_New(name, id);
	RedisModule_Free(name);

	Index *idx = NULL;
	uint index_count = RedisModule_LoadUnsigned(rdb);
	for(uint i = 0; i < index_count; i++) {
		IndexType type = RedisModule_LoadUnsigned(rdb);
		char *field = RedisModule_LoadStringBuffer(rdb, NULL);

		Schema_AddIndex(&idx, s, field, type);
		RedisModule_Free(field);
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
		gc->node_schemas = array_append(gc->node_schemas, _RdbLoadSchema(rdb, SCHEMA_NODE));
	}

	// #Edge schemas
	schema_count = RedisModule_LoadUnsigned(rdb);

	// Load each edge schema
	gc->relation_schemas = array_ensure_cap(gc->relation_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		gc->relation_schemas = array_append(gc->relation_schemas, _RdbLoadSchema(rdb, SCHEMA_EDGE));
	}
}
