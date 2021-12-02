/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v11.h"

static Schema *_RdbLoadSchema(RedisModuleIO *rdb, SchemaType type) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * index type
	 * language if fulltext index
	 * stopword if fullext index
	 * #properties
	 * indexed property X M */

	char *language = NULL;
	char **stopwords = NULL;
	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = Schema_New(type, id, name);
	RedisModule_Free(name);

	uint index_count     = RedisModule_LoadUnsigned(rdb);
	while (index_count > 0) {
		IndexType index_type = RedisModule_LoadUnsigned(rdb);

		if(index_type == IDX_FULLTEXT) {
			char *lang = RedisModule_LoadStringBuffer(rdb, NULL);
			language = rm_strdup(lang);
			RedisModule_Free(lang);
			
			uint stopwords_count = RedisModule_LoadUnsigned(rdb);
			if(stopwords_count > 0) {
				stopwords = array_new(char *, stopwords_count);
				for (uint i = 0; i < stopwords_count; i++) {
					char *stopword = RedisModule_LoadStringBuffer(rdb, NULL);
					array_append(stopwords, rm_strdup(stopword));
					RedisModule_Free(stopword);
				}
			}
		}

		Index *idx = NULL;
		uint fields_count = RedisModule_LoadUnsigned(rdb);
		for(uint i = 0; i < fields_count; i++) {
			char *field_name = RedisModule_LoadStringBuffer(rdb, NULL);
			IndexField field;
			if(index_type == IDX_FULLTEXT) {
				double weight = RedisModule_LoadDouble(rdb);
				bool nostem = RedisModule_LoadUnsigned(rdb);
				char *phonetic = RedisModule_LoadStringBuffer(rdb, NULL);

				if(strcmp(phonetic, "no") == 0) {
					field = IndexField_New(
						rm_strdup(field_name),
						weight,
						nostem,
						INDEX_FIELD_DEFAULT_PHONETIC);
				} else {
					field = IndexField_New(
						rm_strdup(field_name),
						weight,
						nostem,
						rm_strdup(phonetic));
				}
				RedisModule_Free(phonetic);
			} else {
				field = IndexField_New(
					rm_strdup(field_name),
					INDEX_FIELD_DEFAULT_WEIGHT,
					INDEX_FIELD_DEFAULT_NOSTEM,
					INDEX_FIELD_DEFAULT_PHONETIC);
			}
			if(type == SCHEMA_NODE || (strcmp(field.name, "_src_id") != 0 && strcmp(field.name, "_dest_id") != 0)) {
				Schema_AddIndex(&idx, s, &field, index_type);
			} else {
				printf("\n%s\n", field.name);
				rm_free(field.name);
			}
			RedisModule_Free(field_name);
		}

		if(index_type == IDX_FULLTEXT) {
			idx->language = language;
			idx->stopwords = stopwords;
		}

		index_count -= fields_count;
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

void RdbLoadGraphSchema_v11(RedisModuleIO *rdb, GraphContext *gc) {
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
