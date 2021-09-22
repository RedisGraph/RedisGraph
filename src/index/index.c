/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
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

extern void populateEdgeIndex(Index *idx); 
extern void populateNodeIndex(Index *idx);

// TODO: Remove this comment when https://github.com/RediSearch/RediSearch/issues/1100 is closed
// static int _getNodeAttribute(void *ctx, const char *fieldName, const void *id, char **strVal,
// 							 double *doubleVal) {
// 	Node n = GE_NEW_NODE();
// 	NodeID nId = *(NodeID *)id;
// 	GraphContext *gc = (GraphContext *)ctx;
// 	Graph *g = gc->g;

// 	int node_found = Graph_GetNode(g, nId, &n);
// 	UNUSED(node_found);
// 	ASSERT(node_found != 0);

// 	Attribute_ID attrId = GraphContext_GetAttributeID(gc, fieldName);
// 	SIValue *v = GraphEntity_GetProperty((GraphEntity *)&n, attrId);
// 	int ret;
// 	if(v == PROPERTY_NOTFOUND) {
// 		ret = RSVALTYPE_NOTFOUND;
// 	} else if(v->type & T_STRING) {
// 		*strVal = v->stringval;
// 		ret = RSVALTYPE_STRING;
// 	} else if(v->type & SI_NUMERIC) {
// 		*doubleVal = SI_GET_NUMERIC(*v);
// 		ret = RSVALTYPE_DOUBLE;
// 	} else {
// 		// Skiping booleans.
// 		ret = RSVALTYPE_NOTFOUND;
// 	}
// 	return ret;
// }

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
	const char  *lang            = NULL;  // default language
	const char  *field_name      = NULL;  // name of current indexed field
	SIValue     *v               = NULL;  // current indexed value
	RSIndex     *rsIdx           = idx->idx;
	EntityID    id               = ENTITY_GET_ID(e);

	*doc_field_count = 0;

	// list of none indexable fields
	uint none_indexable_fields_count = 0; // number of none indexed fields
	const char *none_indexable_fields[idx->fields_count]; // none indexed fields

	// create a document out of node
	RSDoc *doc = RediSearch_CreateDocument(key, key_len, score, lang);

	// add document field for each indexed property
	if(idx->type == IDX_FULLTEXT) {
		for(uint i = 0; i < idx->fields_count; i++) {
			field_name = idx->fields[i];
			v = GraphEntity_GetProperty(e, idx->fields_ids[i]);
			if(v == PROPERTY_NOTFOUND) continue;

			SIType t = SI_TYPE(*v);

			// value must be of type string
			if(t == T_STRING) {
				*doc_field_count += 1;
				RediSearch_DocumentAddFieldString(doc, idx->fields[i], 
						v->stringval, strlen(v->stringval), RSFLDTYPE_FULLTEXT);
			}
		}
	} else {
		for(uint i = 0; i < idx->fields_count; i++) {
			field_name = idx->fields[i];
			v = GraphEntity_GetProperty(e, idx->fields_ids[i]);
			if(v == PROPERTY_NOTFOUND) continue;

			SIType t = SI_TYPE(*v);

			*doc_field_count += 1;
			if(t == T_STRING) {
				RediSearch_DocumentAddFieldString(doc, idx->fields[i],
						v->stringval, strlen(v->stringval), RSFLDTYPE_TAG);
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
	idx->fields        =  array_new(char *, 0);
	idx->label_id      =  label_id;
	idx->fields_ids    =  array_new(Attribute_ID, 0);
	idx->entity_type   =  entity_type;
	idx->fields_count  =  0;

	return idx;
}

// adds field to index
void Index_AddField
(
	Index *idx,
	const char *field
) {
	ASSERT(idx != NULL);

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Attribute_ID fieldID = GraphContext_FindOrAddAttribute(gc, field);
	if(Index_ContainsAttribute(idx, fieldID)) return;

	idx->fields_count++;
	array_append(idx->fields, rm_strdup(field));
	array_append(idx->fields_ids, fieldID);
}

// removes fields from index
void Index_RemoveField
(
	Index *idx,
	const char *field
) {
	ASSERT(idx != NULL);

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Attribute_ID attribute_id = GraphContext_FindOrAddAttribute(gc, field);
	if(!Index_ContainsAttribute(idx, attribute_id)) return;

	for(uint i = 0; i < idx->fields_count; i++) {
		if(idx->fields_ids[i] == attribute_id) {
			idx->fields_count--;
			rm_free(idx->fields[i]);
			array_del_fast(idx->fields, i);
			array_del_fast(idx->fields_ids, i);
			break;
		}
	}
}

// constructs index
void Index_Construct
(
	Index *idx
) {
	ASSERT(idx != NULL);

	// RediSearch index already exists, re-construct
	if(idx->idx) {
		RediSearch_DropIndex(idx->idx);
		idx->idx = NULL;
	}

	RSIndex *rsIdx = NULL;
	RSIndexOptions *idx_options = RediSearch_CreateIndexOptions();
	// TODO: Remove this comment when https://github.com/RediSearch/RediSearch/issues/1100 is closed
	// RediSearch_IndexOptionsSetGetValueCallback(idx_options, _getNodeAttribute, gc);

	// enable GC, every 30 seconds gc will check if there's garbage
	// if there are over 100 docs to remove GC will perform clean up
	RediSearch_IndexOptionsSetGCPolicy(idx_options, GC_POLICY_FORK);
	rsIdx = RediSearch_CreateIndex(idx->label, idx_options);
	RediSearch_FreeIndexOptions(idx_options);

	// create indexed fields
	if(idx->type == IDX_FULLTEXT) {
		for(uint i = 0; i < idx->fields_count; i++) {
			// introduce text field
			RediSearch_CreateTextField(rsIdx, idx->fields[i]);
		}
	} else {
		for(uint i = 0; i < idx->fields_count; i++) {
			// introduce both text, numeric and geo fields
			unsigned types = RSFLDTYPE_NUMERIC | RSFLDTYPE_GEO | RSFLDTYPE_TAG;
			RSFieldID fieldID = RediSearch_CreateField(rsIdx, idx->fields[i],
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
	if(idx->entity_type == GETYPE_NODE) populateNodeIndex(idx);
	else populateEdgeIndex(idx);
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

	return idx->fields_count;
}

// returns indexed fields
const char **Index_GetFields
(
	const Index *idx
) {
	ASSERT(idx != NULL);

	return (const char **)idx->fields;
}

bool Index_ContainsAttribute
(
	const Index *idx,
	Attribute_ID attribute_id
) {
	ASSERT(idx != NULL);

	if(attribute_id == ATTRIBUTE_NOTFOUND) return false;

	for(uint i = 0; i < idx->fields_count; i++) {
		if(idx->fields_ids[i] == attribute_id) return true;
	}

	return false;
}

// free index
void Index_Free
(
	Index *idx
) {
	ASSERT(idx != NULL);

	if(idx->idx) RediSearch_DropIndex(idx->idx);

	rm_free(idx->label);

	for(uint i = 0; i < idx->fields_count; i++) rm_free(idx->fields[i]);

	array_free(idx->fields);
	array_free(idx->fields_ids);

	rm_free(idx);
}

