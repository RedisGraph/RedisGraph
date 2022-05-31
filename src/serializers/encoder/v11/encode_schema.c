/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_v11.h"

static void _RdbSaveAttributeKeys
(
	IOEncoder *io,
	GraphContext *gc
) {
	/* Format:
	 * #attribute keys
	 * attribute keys
	*/

	uint count = GraphContext_AttributeCount(gc);
	IOEncoder_SaveUnsigned(io, count);
	for(uint i = 0; i < count; i ++) {
		char *key = gc->string_mapping[i];
		IOEncoder_SaveStringBuffer(io, key, strlen(key) + 1);
	}
}

static inline void _RdbSaveFullTextIndexData
(
	IOEncoder *io,
	Index *idx
) {
	/* Format:
	 * language
	 * #stopwords - N
	 * N * stopword
	 * #properties - M
	 * M * property {name, weight, nostem, phonetic} */

	// encode language
	const char *language = Index_GetLanguage(idx);
	IOEncoder_SaveStringBuffer(io, language, strlen(language) + 1);

	size_t stopwords_count;
	char **stopwords = Index_GetStopwords(idx, &stopwords_count);
	// encode stopwords count
	IOEncoder_SaveUnsigned(io, stopwords_count);
	for (size_t i = 0; i < stopwords_count; i++) {
		char *stopword = stopwords[i];
		IOEncoder_SaveStringBuffer(io, stopword, strlen(stopword) + 1);
		rm_free(stopword);
	}
	rm_free(stopwords);

	// encode field count
	uint fields_count = Index_FieldsCount(idx);
	IOEncoder_SaveUnsigned(io, fields_count);
	for(uint i = 0; i < fields_count; i++) {
		// encode field
		IOEncoder_SaveStringBuffer(io, idx->fields[i].name, strlen(idx->fields[i].name) + 1);
		IOEncoder_SaveDouble(io, idx->fields[i].weight);
		IOEncoder_SaveUnsigned(io, idx->fields[i].nostem);
		IOEncoder_SaveStringBuffer(io, idx->fields[i].phonetic, 
			strlen(idx->fields[i].phonetic) + 1);
	}
}

static inline void _RdbSaveExactMatchIndex
(
	IOEncoder *io,
	SchemaType type,
	Index *idx
) {
	/* Format:
	 * #properties - M
	 * M * property */

	uint fields_count = Index_FieldsCount(idx);

	// for exact-match index on an edge type, decrease `fields_count` by 2
	// skipping `_src_id` and `_dest_id` fields
	uint encode_fields_count =
		type == SCHEMA_EDGE ? fields_count - 2 : fields_count;

	// encode field count
	IOEncoder_SaveUnsigned(io, encode_fields_count);
	for(uint i = 0; i < fields_count; i++) {
		char *field_name = idx->fields[i].name;

		// for exact-match index on an edge type, skip both `_src_id` and
		// `_dest_id` fields, these are introduce automaticly by the index
		// construct routine
		if(type == SCHEMA_EDGE && 
		   (strcmp(field_name, "_src_id") == 0 ||
		    strcmp(field_name, "_dest_id") == 0)) continue;

		// encode field
		IOEncoder_SaveStringBuffer(io, field_name, strlen(field_name) + 1);
	}
}

static inline void _RdbSaveIndexData
(
	IOEncoder *io,
	SchemaType type,
	Index *idx
) {
	/* Format:
	 * type
	 * index data */

	if(!idx) return;

	// index type
	IOEncoder_SaveUnsigned(io, idx->type);

	if(idx->type == IDX_FULLTEXT) _RdbSaveFullTextIndexData(io, idx);
	else _RdbSaveExactMatchIndex(io, type, idx);
}

static void _RdbSaveSchema(IOEncoder *io, Schema *s) {
	/* Format:
	 * id
	 * name
	 * #indices
	 * (index type, indexed property) X M */

	// Schema ID.
	IOEncoder_SaveUnsigned(io, s->id);

	// Schema name.
	IOEncoder_SaveStringBuffer(io, s->name, strlen(s->name) + 1);

	// Number of indices.
	IOEncoder_SaveUnsigned(io, Schema_IndexCount(s));

	// Exact match indices.
	_RdbSaveIndexData(io, s->type, s->index);

	// Fulltext indices.
	_RdbSaveIndexData(io, s->type, s->fulltextIdx);
}

void RdbSaveGraphSchema_v11
(
	IOEncoder *io,
	GraphContext *gc
) {
	/* Format:
	 * attribute keys (unified schema)
	 * #node schemas
	 * node schema X #node schemas
	 * #relation schemas
	 * relation schema X #relation schemas
	*/

	// Serialize all attribute keys
	_RdbSaveAttributeKeys(io, gc);

	// #Node schemas.
	unsigned short schema_count = GraphContext_SchemaCount(gc, SCHEMA_NODE);
	IOEncoder_SaveUnsigned(io, schema_count);

	// Name of label X #node schemas.
	for(int i = 0; i < schema_count; i++) {
		Schema *s = gc->node_schemas[i];
		_RdbSaveSchema(io, s);
	}

	// #Relation schemas.
	unsigned short relation_count = GraphContext_SchemaCount(gc, SCHEMA_EDGE);
	IOEncoder_SaveUnsigned(io, relation_count);

	// Name of label X #relation schemas.
	for(unsigned short i = 0; i < relation_count; i++) {
		Schema *s = gc->relation_schemas[i];
		_RdbSaveSchema(io, s);
	}
}

