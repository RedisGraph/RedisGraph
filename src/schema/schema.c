/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "schema.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"

Schema *Schema_New(const char *name, int id) {
	Schema *schema = rm_malloc(sizeof(Schema));
	schema->id = id;
	schema->index = NULL;
	schema->fulltextIdx = NULL;
	schema->name = rm_strdup(name);
	return schema;
}

const char *Schema_GetName(const Schema *s) {
	ASSERT(s);
	return s->name;
}

bool Schema_HasIndices(const Schema *s) {
	ASSERT(s);
	return (s->fulltextIdx || s->index);
}

unsigned short Schema_IndexCount(const Schema *s) {
	ASSERT(s);
	unsigned short n = 0;

	if(s->index) n += Index_FieldsCount(s->index);
	if(s->fulltextIdx) n += Index_FieldsCount(s->fulltextIdx);

	return n;
}

Index *Schema_GetIndex(const Schema *s, Attribute_ID *attribute_id, IndexType type) {
	Index *idx = NULL;

	if(type == IDX_EXACT_MATCH) {
		idx = s->index;
	} else if(type ==  IDX_FULLTEXT) {
		idx = s->fulltextIdx;
	} else {
		// If type is unspecified, use the first index that exists.
		idx = s->index ? : s->fulltextIdx;
	}

	if(!idx) return NULL;

	// Make sure field is indexed.
	if(attribute_id) {
		if(!Index_ContainsAttribute(idx, *attribute_id)) return NULL;
	}

	return idx;
}

int Schema_AddIndex(Index **idx, Schema *s, const char *field, IndexType type) {
	ASSERT(field);

	Index *_idx = Schema_GetIndex(s, NULL, type);

	// Index exists, make sure attribute isn't already indexed.
	if(_idx != NULL) {
		GraphContext *gc = QueryCtx_GetGraphCtx();
		Attribute_ID fieldID = GraphContext_FindOrAddAttribute(gc, field);
		if(Index_ContainsAttribute(_idx, fieldID)) return INDEX_FAIL;
	} else {
		// Index doesn't exist, create it.
		_idx = Index_New(s->name, type);
		if(type == IDX_FULLTEXT) s->fulltextIdx = _idx;
		else s->index = _idx;
	}

	Index_AddField(_idx, field);

	*idx = _idx;
	return INDEX_OK;
}

static int _Schema_RemoveExactMatchIndex(Schema *s, const char *field) {
	ASSERT(field != NULL);
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Attribute_ID attribute_id = GraphContext_GetAttributeID(gc, field);
	if(attribute_id == ATTRIBUTE_NOTFOUND) return INDEX_FAIL;

	Index *idx = Schema_GetIndex(s, &attribute_id, IDX_EXACT_MATCH);
	if(idx == NULL) return INDEX_FAIL;

	Index_RemoveField(idx, field);

	// if index field count dropped to 0, remove index from schema
	if(Index_FieldsCount(idx) == 0) {
		Index_Free(idx);
		s->index = NULL;
	}

	return INDEX_OK;
}

static int _Schema_RemoveFullTextIndex(Schema *s) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx == NULL) return INDEX_FAIL;

	Index_Free(idx);
	s->fulltextIdx = NULL;

	return INDEX_OK;
}

int Schema_RemoveIndex(Schema *s, const char *field, IndexType type) {
	switch(type) {
	case IDX_FULLTEXT:
		return _Schema_RemoveFullTextIndex(s);
	case IDX_EXACT_MATCH:
		return _Schema_RemoveExactMatchIndex(s, field);
	default:
		return INDEX_FAIL;
	}
}

// Index node under all schema indices.
void Schema_AddNodeToIndices(const Schema *s, const Node *n) {
	if(!s) return;
	Index *idx = NULL;

	idx = s->fulltextIdx;
	if(idx) Index_IndexNode(idx, n);

	idx = s->index;
	if(idx) Index_IndexNode(idx, n);
}

void Schema_Free(Schema *schema) {
	if(schema->name) rm_free(schema->name);

	// Free indicies.
	if(schema->index) Index_Free(schema->index);
	if(schema->fulltextIdx) Index_Free(schema->fulltextIdx);
	rm_free(schema);
}

