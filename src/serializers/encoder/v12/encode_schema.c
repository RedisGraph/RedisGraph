/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "encode_v12.h"

static void _RdbSaveAttributeKeys
(
	RedisModuleIO *rdb,
	GraphContext *gc
) {
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

static inline void _RdbSaveFullTextIndexData
(
	RedisModuleIO *rdb,
	Index idx
) {
	/* Format:
	 * language
	 * #stopwords - N
	 * N * stopword
	 * #properties - M
	 * M * property {name, weight, nostem, phonetic} */

	// encode language
	const char *language = Index_GetLanguage(idx);
	RedisModule_SaveStringBuffer(rdb, language, strlen(language) + 1);

	size_t stopwords_count;
	char **stopwords = Index_GetStopwords(idx, &stopwords_count);
	// encode stopwords count
	RedisModule_SaveUnsigned(rdb, stopwords_count);
	for (size_t i = 0; i < stopwords_count; i++) {
		char *stopword = stopwords[i];
		RedisModule_SaveStringBuffer(rdb, stopword, strlen(stopword) + 1);
		rm_free(stopword);
	}
	rm_free(stopwords);

	// encode field count
	uint fields_count = Index_FieldsCount(idx);
	const IndexField *fields = Index_GetFields(idx);

	RedisModule_SaveUnsigned(rdb, fields_count);
	for(uint i = 0; i < fields_count; i++) {
		// encode field
		const IndexField *f = fields + i;
		RedisModule_SaveStringBuffer(rdb, f->name, strlen(f->name) + 1);
		RedisModule_SaveDouble(rdb, f->weight);
		RedisModule_SaveUnsigned(rdb, f->nostem);
		RedisModule_SaveStringBuffer(rdb, f->phonetic, strlen(f->phonetic) + 1);
	}
}

static inline void _RdbSaveExactMatchIndex
(
	RedisModuleIO *rdb,
	SchemaType type,
	Index idx
) {
	/* Format:
	 * #properties - M
	 * M * property */

	uint fields_count = Index_FieldsCount(idx);
	const IndexField *fields = Index_GetFields(idx);

	// encode field count
	RedisModule_SaveUnsigned(rdb, fields_count);
	for(uint i = 0; i < fields_count; i++) {
		char *field_name = fields[i].name;

		// encode field
		RedisModule_SaveStringBuffer(rdb, field_name, strlen(field_name) + 1);
	}
}

static inline void _RdbSaveIndexData
(
	RedisModuleIO *rdb,
	SchemaType type,
	Index idx
) {
	/* Format:
	 * type
	 * index data */

	if(!idx) return;

	// index type
	IndexType t = Index_Type(idx);
	ASSERT(t == IDX_EXACT_MATCH || t == IDX_FULLTEXT);

	RedisModule_SaveUnsigned(rdb, t);

	if(t == IDX_FULLTEXT) {
		_RdbSaveFullTextIndexData(rdb, idx);
	} else {
		_RdbSaveExactMatchIndex(rdb, type, idx);
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
	_RdbSaveIndexData(rdb, s->type, s->index);

	// Fulltext indices.
	_RdbSaveIndexData(rdb, s->type, s->fulltextIdx);
}

void RdbSaveGraphSchema_v12(RedisModuleIO *rdb, GraphContext *gc) {
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

