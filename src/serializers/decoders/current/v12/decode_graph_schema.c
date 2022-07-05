/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_v12.h"

static void _RdbLoadFullTextIndex
(
	IODecoder *io,
	GraphContext *gc,
	Schema *s,
	bool already_loaded
) {
	/* Format:
	 * language
	 * #stopwords - N
	 * N * stopword
	 * #properties - M
	 * M * property: {name, weight, nostem, phonetic} */

	Index *idx       = NULL;
	char *language   = IODecoder_LoadStringBuffer(io, NULL);
	char **stopwords = NULL;
	
	uint stopwords_count = IODecoder_LoadUnsigned(io);
	if(stopwords_count > 0) {
		stopwords = array_new(char *, stopwords_count);
		for (uint i = 0; i < stopwords_count; i++) {
			char *stopword = IODecoder_LoadStringBuffer(io, NULL);
			array_append(stopwords, stopword);
		}
	}

	uint fields_count = IODecoder_LoadUnsigned(io);
	for(uint i = 0; i < fields_count; i++) {
		char    *field_name  =  IODecoder_LoadStringBuffer(io, NULL);
		double  weight       =  IODecoder_LoadDouble(io);
		bool    nostem       =  IODecoder_LoadUnsigned(io);
		char    *phonetic    =  IODecoder_LoadStringBuffer(io, NULL);

		if(!already_loaded) {
			IndexField field;
			Attribute_ID field_id = GraphContext_FindOrAddAttribute(gc, field_name);
			IndexField_New(&field, field_id, field_name, weight, nostem, phonetic);
			Schema_AddIndex(&idx, s, &field, IDX_FULLTEXT);
		}

		RedisModule_Free(field_name);
		RedisModule_Free(phonetic);
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
	IODecoder *io,
	GraphContext *gc,
	Schema *s,
	bool already_loaded
) {
	/* Format:
	 * #properties - M
	 * M * property */

	Index *idx = NULL;
	uint fields_count = IODecoder_LoadUnsigned(io);
	for(uint i = 0; i < fields_count; i++) {
		char *field_name = IODecoder_LoadStringBuffer(io, NULL);
		if(!already_loaded) {
			IndexField field;
			Attribute_ID field_id = GraphContext_FindOrAddAttribute(gc, field_name);
			IndexField_New(&field, field_id, field_name, INDEX_FIELD_DEFAULT_WEIGHT,
				INDEX_FIELD_DEFAULT_NOSTEM, INDEX_FIELD_DEFAULT_PHONETIC);

			Schema_AddIndex(&idx, s, &field, IDX_EXACT_MATCH);
		}
		RedisModule_Free(field_name);
	}
}

static Schema *_RdbLoadSchema
(
	IODecoder *io,
	GraphContext *gc,
	SchemaType type,
	bool already_loaded
) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * index type
	 * index data */

	int id = IODecoder_LoadUnsigned(io);
	char *name = IODecoder_LoadStringBuffer(io, NULL);
	Schema *s = already_loaded ? NULL : Schema_New(type, id, name);
	RedisModule_Free(name);

	uint index_count = IODecoder_LoadUnsigned(io);
	for (uint index = 0; index < index_count; index++) {
		IndexType index_type = IODecoder_LoadUnsigned(io);

		switch(index_type) {
			case IDX_FULLTEXT:
				_RdbLoadFullTextIndex(io, gc, s, already_loaded);
				break;
			case IDX_EXACT_MATCH:
				_RdbLoadExactMatchIndex(io, gc, s, already_loaded);
				break;
			default:
				ASSERT(false);
				break;
		}
	}

	if(s) {
		// no entities are expected to be in the graph in this point in time
		if(s->index) Index_Construct(s->index, gc->g);
		if(s->fulltextIdx) Index_Construct(s->fulltextIdx, gc->g);
	}

	return s;
}

static void _RdbLoadAttributeKeys
(
	IODecoder *io,
	GraphContext *gc
) {
	/* Format:
	 * #attribute keys
	 * attribute keys
	 */

	uint count = IODecoder_LoadUnsigned(io);
	for(uint i = 0; i < count; i ++) {
		char *attr = IODecoder_LoadStringBuffer(io, NULL);
		GraphContext_FindOrAddAttribute(gc, attr);
		RedisModule_Free(attr);
	}
}

void RdbLoadGraphSchema_v12
(
	IODecoder *io,
	GraphContext *gc
) {
	/* Format:
	 * attribute keys (unified schema)
	 * #node schemas
	 * node schema X #node schemas
	 * #relation schemas
	 * unified relation schema
	 * relation schema X #relation schemas
	 */

	// attributes, Load the full attribute mapping
	_RdbLoadAttributeKeys(io, gc);

	// #node schemas
	uint schema_count = IODecoder_LoadUnsigned(io);

	bool already_loaded = array_len(gc->node_schemas) > 0;

	// load each node schema
	gc->node_schemas = array_ensure_cap(gc->node_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		Schema *s = _RdbLoadSchema(io, gc, SCHEMA_NODE, already_loaded);
		if(!already_loaded) array_append(gc->node_schemas, s);
	}

	// #edge schemas
	schema_count = IODecoder_LoadUnsigned(io);

	// load each edge schema
	gc->relation_schemas = array_ensure_cap(gc->relation_schemas, schema_count);
	for(uint i = 0; i < schema_count; i ++) {
		Schema *s = _RdbLoadSchema(io, gc, SCHEMA_EDGE, already_loaded);
		if(!already_loaded) array_append(gc->relation_schemas, s);
	}
}

