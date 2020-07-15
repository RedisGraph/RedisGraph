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
#include <assert.h>

Schema *Schema_New(const char *name, int id) {
	Schema *schema = rm_malloc(sizeof(Schema));
	schema->id = id;
	schema->index = NULL;
	schema->fulltextIdx = NULL;
	schema->name = rm_strdup(name);
	return schema;
}

const char *Schema_GetName(const Schema *s) {
	assert(s);
	return s->name;
}

bool Schema_HasIndices(const Schema *s) {
	assert(s);
	return (s->fulltextIdx || s->index);
}

unsigned short Schema_IndexCount(const Schema *s) {
	assert(s);
	unsigned short n = 0;

	if(s->index) n += Index_FieldsCount(s->index);
	if(s->fulltextIdx) n += Index_FieldsCount(s->fulltextIdx);

	return n;
}

Index *Schema_GetIndex(const Schema *s, const char *field, IndexType type) {
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
	if(field) {
		if(!Index_ContainsField(idx, field)) return NULL;
	}

	return idx;
}

int Schema_AddIndex(Index **idx, Schema *s, const char *field, IndexType type) {
	assert(field);

	*idx = NULL;
	Index *_idx = Schema_GetIndex(s, NULL, type);

	// Index exists, make sure attribute isn't already indexed.
	if(_idx != NULL) {
		if(Index_ContainsField(_idx, field)) return INDEX_FAIL;
	}

	// Index doesn't exists, create it.
	if(!_idx) {
		_idx = Index_New(s->name, type);
		if(type == IDX_FULLTEXT) s->fulltextIdx = _idx;
		else s->index = _idx;
	}

	Index_AddField(_idx, field);

	*idx = _idx;
	return INDEX_OK;
}

int Schema_RemoveIndex(Schema *s, const char *field, IndexType type) {
	Index *idx = Schema_GetIndex(s, field, type);
	if(idx == NULL) return INDEX_FAIL;

	type = idx->type;

	// Currently dropping a full-text index doesn't take into account fields.
	if(type == IDX_FULLTEXT) {
		assert(field == NULL);
		Index_Free(idx);
		s->fulltextIdx = NULL;
	} else {
		// Index is of type IDX_EXACT_MATCH
		assert(type == IDX_EXACT_MATCH);
		Index_RemoveField(idx, field);

		// If index field count dropped to 0, remove index from schema.
		if(Index_FieldsCount(idx) == 0) {
			Index_Free(idx);
			s->index = NULL;
		}
	}

	return INDEX_OK;
}

// Index node under all shcema indicies.
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

