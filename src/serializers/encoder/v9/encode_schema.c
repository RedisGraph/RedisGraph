/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v9.h"

static void _RdbSaveAttributeKeys(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * #attribute keys
	 * attribute keys
	*/

	uint count = GraphContext_AttributeCount(gc);
	RedisModule_SaveUnsigned(rdb, count);
	for(uint i = 0; i < count; i ++) {
		char *key = gc->string_mapping[i];
		RedisModule_SaveStringBuffer(rdb, key, strlen(key) + 1);
	}
}

static inline void _RdbSaveIndexData(RedisModuleIO *rdb, Index *idx) {
	if(!idx) return;

	// Index type
	RedisModule_SaveUnsigned(rdb, idx->type);

	if(idx->type == IDX_FULLTEXT) {
		// Index language
		char *language = idx->language ? idx->language : "english";
		RedisModule_SaveStringBuffer(rdb, language, strlen(language));

		uint stopwords_count = idx->stopwords ? array_len(idx->stopwords) : 0;
		// Index stopwords count
		RedisModule_SaveUnsigned(rdb, stopwords_count);
		for (uint i = 0; i < stopwords_count; i++) {
			char *stopword = idx->stopwords[i];
			// Index stopword
			RedisModule_SaveStringBuffer(rdb, stopword, strlen(stopword));
		}
	}

	uint fields_count = Index_FieldsCount(idx);
	// Indexed fields count
	RedisModule_SaveUnsigned(rdb, fields_count);
	for(uint i = 0; i < fields_count; i++) {
		// Indexed property
		RedisModule_SaveStringBuffer(rdb, idx->fields[i], strlen(idx->fields[i]) + 1);
	}
}

static void _RdbSaveSchema(RedisModuleIO *rdb, Schema *s) {
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

void RdbSaveGraphSchema_v9(RedisModuleIO *rdb, GraphContext *gc) {
	/* Format:
	 * attribute keys (unified schema)
	 * #node schemas
	 * node schema X #node schemas
	 * #relation schemas
	 * relation schema X #relation schemas
	*/

	// Serialize all attribute keys
	_RdbSaveAttributeKeys(rdb, gc);

	// #Node schemas.
	unsigned short schema_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
	RedisModule_SaveUnsigned(rdb, schema_count);

	// Name of label X #node schemas.
	for(int i = 0; i < schema_count; i++) {
		Schema *s = gc->node_schemas[i];
		_RdbSaveSchema(rdb, s);
	}

	// #Relation schemas.
	unsigned short relation_count = GraphContext_SchemaCount(gc, SCHEMA_EDGE);
	RedisModule_SaveUnsigned(rdb, relation_count);

	// Name of label X #relation schemas.
	for(unsigned short i = 0; i < relation_count; i++) {
		Schema *s = gc->relation_schemas[i];
		_RdbSaveSchema(rdb, s);
	}
}
