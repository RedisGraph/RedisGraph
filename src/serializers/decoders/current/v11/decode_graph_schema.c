/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v11.h"

static void _RdbLoadFullTextIndex
(
	RedisModuleIO *rdb,
	Schema *s,
	bool already_loaded
) {
	/* Format:
	 * language
	 * #stopwords - N
	 * N * stopword
	 * #properties - M
	 * M * property */

	Index *idx       = NULL;
	char *language   = RedisModule_LoadStringBuffer(rdb, NULL);
	char **stopwords = NULL;
	
	uint stopwords_count = RedisModule_LoadUnsigned(rdb);
	if(stopwords_count > 0) {
		stopwords = array_new(char *, stopwords_count);
		for (uint i = 0; i < stopwords_count; i++) {
			char *stopword = RedisModule_LoadStringBuffer(rdb, NULL);
			array_append(stopwords, stopword);
		}
	}

	uint fields_count = RedisModule_LoadUnsigned(rdb);
	for(uint i = 0; i < fields_count; i++) {
		char *field_name = RedisModule_LoadStringBuffer(rdb, NULL);
		IndexField field;
		double weight = RedisModule_LoadDouble(rdb);
		bool nostem = RedisModule_LoadUnsigned(rdb);
		char *phonetic = RedisModule_LoadStringBuffer(rdb, NULL);

		IndexField_New(&field, field_name, weight, nostem, phonetic);
		RedisModule_Free(phonetic);

		// in case of decoding edge index _src_id and _dest_id fields added by default
		if(s->type == SCHEMA_NODE || (strcmp(field.name, "_src_id") != 0 && strcmp(field.name, "_dest_id") != 0)) {
			if(!already_loaded) Schema_AddIndex(&idx, s, &field, IDX_FULLTEXT);
		} else {
			rm_free(field.name);
		}
		RedisModule_Free(field_name);
	}

	if(!already_loaded) {
		ASSERT(idx != NULL);
		Index_SetLanguage(idx, language);
		Index_SetStopwords(idx, stopwords);
	}
	
	// free language
	RedisModule_Free(language);

	// free stopwords
	for (uint i = 0; i < stopwords_count; i++) RedisModule_Free(stopwords[i]);
	array_free(stopwords);
}

static void _RdbLoadExactMatchIndex
(
	RedisModuleIO *rdb,
	Schema *s,
	bool already_loaded
) {
	/* Format:
	 * #properties - M
	 * M * property */

	Index *idx = NULL;
	uint fields_count = RedisModule_LoadUnsigned(rdb);
	for(uint i = 0; i < fields_count; i++) {
		char *field_name = RedisModule_LoadStringBuffer(rdb, NULL);
		IndexField field;
		IndexField_New(&field, field_name, INDEX_FIELD_DEFAULT_WEIGHT,
			INDEX_FIELD_DEFAULT_NOSTEM, INDEX_FIELD_DEFAULT_PHONETIC);

		// in case of decoding edge index _src_id and _dest_id fields added by default
		if(s->type == SCHEMA_NODE || (strcmp(field.name, "_src_id") != 0 && strcmp(field.name, "_dest_id") != 0)) {
			if(!already_loaded) Schema_AddIndex(&idx, s, &field, IDX_EXACT_MATCH);
		} else {
			rm_free(field.name);
		}
		RedisModule_Free(field_name);
	}
}

static Schema *_RdbLoadSchema
(
	RedisModuleIO *rdb,
	SchemaType type,
	bool already_loaded
) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * index type
	 * index data */

	int id = RedisModule_LoadUnsigned(rdb);
	char *name = RedisModule_LoadStringBuffer(rdb, NULL);
	Schema *s = already_loaded ? NULL : Schema_New(type, id, name);
	RedisModule_Free(name);

	uint index_count = RedisModule_LoadUnsigned(rdb);
	for (uint index = 0; index < index_count; index++) {
		IndexType index_type = RedisModule_LoadUnsigned(rdb);

		switch(index_type) {
			case IDX_FULLTEXT:
				_RdbLoadFullTextIndex(rdb, s, already_loaded);
				break;
			case IDX_EXACT_MATCH:
				_RdbLoadExactMatchIndex(rdb, s, already_loaded);
				break;
			default:
				ASSERT(false);
				break;
		}
	}

	if(s) {
		// no entities are expected to be in the graph in this point in time
		if(s->index) Index_Construct(s->index);
		if(s->fulltextIdx) Index_Construct(s->fulltextIdx);
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

	bool already_loaded = array_len(gc->node_schemas) > 0;

	// Load each node schema
	gc->node_schemas = array_ensure_cap(gc->node_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		Schema *s = _RdbLoadSchema(rdb, SCHEMA_NODE, already_loaded);
		if(!already_loaded) array_append(gc->node_schemas, s);
	}

	// #Edge schemas
	schema_count = RedisModule_LoadUnsigned(rdb);

	// Load each edge schema
	gc->relation_schemas = array_ensure_cap(gc->relation_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		Schema *s = _RdbLoadSchema(rdb, SCHEMA_EDGE, already_loaded);
		if(!already_loaded) array_append(gc->relation_schemas, s);
	}
}
