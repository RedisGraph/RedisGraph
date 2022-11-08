/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "index.h"
#include "../value.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../datatypes/point.h"
#include "../graph/graphcontext.h"
#include "../graph/entities/node.h"
#include "../graph/rg_matrix/rg_matrix_iter.h"

extern void populateEdgeIndex(Index *idx, Graph *g); 
extern void populateNodeIndex(Index *idx, Graph *g);

RSDoc *Index_IndexGraphEntity
(
	Index *idx,
	const GraphEntity *e,
	const void *key,
	size_t key_len,
	uint *doc_field_count
) {
	ASSERT(idx              !=  NULL);
	ASSERT(e                !=  NULL);
	ASSERT(key              !=  NULL);
	ASSERT(doc_field_count  !=  NULL);
	ASSERT(key_len          >   0);

	double      score            = 1;     // default score
	IndexField  *field           = NULL;  // current indexed field
	SIValue     *v               = NULL;  // current indexed value
	RSIndex     *rsIdx           = idx->idx;
	EntityID    id               = ENTITY_GET_ID(e);
	uint        field_count      = array_len(idx->fields);

	*doc_field_count = 0;

	// list of none indexable fields
	uint none_indexable_fields_count = 0; // number of none indexed fields
	const char *none_indexable_fields[field_count]; // none indexed fields

	// create an empty document
	RSDoc *doc = RediSearch_CreateDocument2(key, key_len, rsIdx, score,
			idx->language);

	// add document field for each indexed property
	if(idx->type == IDX_FULLTEXT) {
		for(uint i = 0; i < field_count; i++) {
			field = idx->fields + i;
			const char *field_name = field->name;
			v = GraphEntity_GetProperty(e, field->id);
			if(v == ATTRIBUTE_NOTFOUND) continue;

			SIType t = SI_TYPE(*v);

			// value must be of type string
			if(t == T_STRING) {
				*doc_field_count += 1;
				RediSearch_DocumentAddFieldString(doc, field_name, v->stringval,
						strlen(v->stringval), RSFLDTYPE_FULLTEXT);
			}
		}
	} else {
		for(uint i = 0; i < field_count; i++) {
			field = idx->fields + i;
			const char *field_name = field->name;
			v = GraphEntity_GetProperty(e, field->id);
			if(v == ATTRIBUTE_NOTFOUND) continue;

			SIType t = SI_TYPE(*v);

			*doc_field_count += 1;
			if(t == T_STRING) {
				RediSearch_DocumentAddFieldString(doc, field_name, v->stringval,
						strlen(v->stringval), RSFLDTYPE_TAG);
			} else if(t & (SI_NUMERIC | T_BOOL)) {
				double d = SI_GET_NUMERIC(*v);
				RediSearch_DocumentAddFieldNumber(doc, field_name, d,
						RSFLDTYPE_NUMERIC);
			} else if(t == T_POINT) {
				double lat = (double)Point_lat(*v);
				double lon = (double)Point_lon(*v);
				RediSearch_DocumentAddFieldGeo(doc, field_name, lat, lon,
						RSFLDTYPE_GEO);
			} else {
				// none indexable field
				none_indexable_fields[none_indexable_fields_count++] =
					field_name;
			}
		}

		// index name of none index fields
		if(none_indexable_fields_count > 0) {
			// concat all none indexable field names
			size_t len = none_indexable_fields_count - 1; // seperators
			for(uint i = 0; i < none_indexable_fields_count; i++) {
				len += strlen(none_indexable_fields[i]);
			}

			char *s = NULL;
			char stack_fields[len];
			if(len < 512) s = stack_fields; // stack base
			else s = rm_malloc(sizeof(char) * len); // heap base

			// concat
			len = sprintf(s, "%s", none_indexable_fields[0]);
			for(uint i = 1; i < none_indexable_fields_count; i++) {
				len += sprintf(s + len, "%c%s", INDEX_SEPARATOR, none_indexable_fields[i]);
			}

			RediSearch_DocumentAddFieldString(doc, INDEX_FIELD_NONE_INDEXED,
						s, len, RSFLDTYPE_TAG);

			// free if heap based
			if(s != stack_fields) rm_free(s);
		}
	}

	return doc;
}

void IndexField_New
(
	IndexField *field,
	Attribute_ID id,
	const char *name,
	double weight,
	bool nostem,
	const char *phonetic
) {
	ASSERT(name      !=  NULL);
	ASSERT(field     !=  NULL);
	ASSERT(phonetic  !=  NULL);

	field->id       = id;
	field->name     = rm_strdup(name);
	field->weight   = weight;
	field->nostem   = nostem;
	field->phonetic = rm_strdup(phonetic);
}

void IndexField_Free
(
	IndexField *field
) {
	ASSERT(field != NULL);

	rm_free(field->name);
	rm_free(field->phonetic);
}

// create a new index
Index *Index_New
(
	const char *label,           // indexed label
	int label_id,                // indexed label id
	IndexType type,              // exact match or full text
	GraphEntityType entity_type  // entity type been indexed
) {
	Index *idx = rm_malloc(sizeof(Index));

	idx->idx           =  NULL;
	idx->type          =  type;
	idx->label         =  rm_strdup(label);
	idx->fields        =  array_new(IndexField, 1);
	idx->label_id      =  label_id;
	idx->language      =  NULL;
	idx->stopwords     =  NULL;
	idx->entity_type   =  entity_type;

	return idx;
}

// adds field to index
void Index_AddField
(
	Index *idx,
	IndexField *field
) {
	ASSERT(idx != NULL);
	ASSERT(field != NULL);

	if(Index_ContainsAttribute(idx, field->id)) {
		IndexField_Free(field);
		return;
	}

	array_append(idx->fields, *field);
}

// removes fields from index
void Index_RemoveField
(
	Index *idx,
	const char *field
) {
	ASSERT(idx != NULL);
	ASSERT(field != NULL);

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Attribute_ID attribute_id = GraphContext_GetAttributeID(gc, field);
	ASSERT(attribute_id != ATTRIBUTE_ID_NONE);

	uint fields_count = array_len(idx->fields);
	for(uint i = 0; i < fields_count; i++) {
		IndexField *field = idx->fields + i;
		if(field->id == attribute_id) {
			// free field
			IndexField_Free(field);
			array_del_fast(idx->fields, i);
			break;
		}
	}
}

// constructs index
void Index_Construct
(
	Index *idx,
	Graph *g
) {
	ASSERT(idx != NULL);

	// RediSearch index already exists, re-construct
	if(idx->idx) {
		RediSearch_DropIndex(idx->idx);
		idx->idx = NULL;
	}

	RSIndex *rsIdx = NULL;
	RSIndexOptions *idx_options = RediSearch_CreateIndexOptions();
	RediSearch_IndexOptionsSetLanguage(idx_options, idx->language);
	// TODO: Remove this comment when https://github.com/RediSearch/RediSearch/issues/1100 is closed
	// RediSearch_IndexOptionsSetGetValueCallback(idx_options, _getNodeAttribute, gc);

	// enable GC, every 30 seconds gc will check if there's garbage
	// if there are over 100 docs to remove GC will perform clean up
	RediSearch_IndexOptionsSetGCPolicy(idx_options, GC_POLICY_FORK);

	if(idx->stopwords) {
		RediSearch_IndexOptionsSetStopwords(idx_options,
				(const char**)idx->stopwords, array_len(idx->stopwords));
	} else if(idx->type == IDX_EXACT_MATCH) {
		RediSearch_IndexOptionsSetStopwords(idx_options, NULL, 0);
	}

	rsIdx = RediSearch_CreateIndex(idx->label, idx_options);
	RediSearch_FreeIndexOptions(idx_options);

	// create indexed fields
	uint fields_count = array_len(idx->fields);
	if(idx->type == IDX_FULLTEXT) {
		for(uint i = 0; i < fields_count; i++) {
			IndexField *field = idx->fields + i;
			// introduce text field
			unsigned options = RSFLDOPT_NONE;
			if(field->nostem) options |= RSFLDOPT_TXTNOSTEM;

			if(strcmp(field->phonetic, INDEX_FIELD_DEFAULT_PHONETIC) != 0) {
				options |= RSFLDOPT_TXTPHONETIC;
			}

			RSFieldID fieldID = RediSearch_CreateField(rsIdx, field->name,
					RSFLDTYPE_FULLTEXT, options);
			RediSearch_TextFieldSetWeight(rsIdx, fieldID, field->weight);
		}
	} else {
		for(uint i = 0; i < fields_count; i++) {
			IndexField *field = idx->fields+i;
			// introduce both text, numeric and geo fields
			unsigned types = RSFLDTYPE_NUMERIC | RSFLDTYPE_GEO | RSFLDTYPE_TAG;
			RSFieldID fieldID = RediSearch_CreateField(rsIdx, field->name,
					types, RSFLDOPT_NONE);

			RediSearch_TagFieldSetSeparator(rsIdx, fieldID, INDEX_SEPARATOR);
			RediSearch_TagFieldSetCaseSensitive(rsIdx, fieldID, 1);
		}

		// for none indexable types e.g. Array introduce an additional field
		// "none_indexable_fields" which will hold a list of attribute names
		// that were not indexed
		RSFieldID fieldID = RediSearch_CreateField(rsIdx,
				INDEX_FIELD_NONE_INDEXED, RSFLDTYPE_TAG, RSFLDOPT_NONE);

		RediSearch_TagFieldSetSeparator(rsIdx, fieldID, INDEX_SEPARATOR);
		RediSearch_TagFieldSetCaseSensitive(rsIdx, fieldID, 1);
	}

	idx->idx = rsIdx;
	if(idx->entity_type == GETYPE_NODE) populateNodeIndex(idx, g);
	else populateEdgeIndex(idx, g);
}

// query index
RSResultsIterator *Index_Query
(
	const Index *idx,
	const char *query,
	char **err
) {
	ASSERT(idx   != NULL);
	ASSERT(query != NULL);

	return RediSearch_IterateQuery(idx->idx, query, strlen(query), err);
}

// returns number of fields indexed
uint Index_FieldsCount
(
	const Index *idx
) {
	ASSERT(idx != NULL);

	return array_len(idx->fields);
}

// returns indexed fields
const IndexField *Index_GetFields
(
	const Index *idx
) {
	ASSERT(idx != NULL);

	return (const IndexField *)idx->fields;
}

bool Index_ContainsAttribute
(
	const Index *idx,
	Attribute_ID attribute_id
) {
	ASSERT(idx != NULL);

	if(attribute_id == ATTRIBUTE_ID_NONE) return false;
	
	uint fields_count = array_len(idx->fields);
	for(uint i = 0; i < fields_count; i++) {
		IndexField *field = idx->fields + i;
		if(field->id == attribute_id) return true;
	}

	return false;
}

int Index_GetLabelID
(
	const Index *idx
) {
	ASSERT(idx != NULL);

	return idx->label_id;
}

const char *Index_GetLanguage
(
	const Index *idx
) {
	return RediSearch_IndexGetLanguage(idx->idx);
}

char **Index_GetStopwords
(
	const Index *idx,
	size_t *size
) {
	if(idx->type == IDX_FULLTEXT)
		return RediSearch_IndexGetStopwords(idx->idx, size);
	
	return NULL;
}

// set indexed language
void Index_SetLanguage
(
	Index *idx,
	const char *language
) {
	ASSERT(idx != NULL);
	ASSERT(language != NULL);
	ASSERT(idx->language == NULL);

	idx->language = rm_strdup(language);
}

// set indexed stopwords
void Index_SetStopwords
(
	Index *idx,
	char **stopwords
) {
	ASSERT(idx != NULL);
	ASSERT(stopwords != NULL);
	ASSERT(idx->stopwords == NULL);

	array_clone_with_cb(idx->stopwords, stopwords, rm_strdup);
}

// free index
void Index_Free(Index *idx) {
	ASSERT(idx != NULL);

	if(idx->idx) RediSearch_DropIndex(idx->idx);

	if(idx->language) rm_free(idx->language);

	uint fields_count = array_len(idx->fields);
	for(uint i = 0; i < fields_count; i++) {
		IndexField_Free(idx->fields + i);
	}
	array_free(idx->fields);

	if(idx->stopwords) {
		uint stopwords_count = array_len(idx->stopwords);
		for(uint i = 0; i < stopwords_count; i++) {
			rm_free(idx->stopwords[i]);
		}
		array_free(idx->stopwords);
	}

	rm_free(idx->label);
	rm_free(idx);
}

