/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "schema.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"

Schema *Schema_New
(
	SchemaType type,
	int id,
	const char *name
) {
	ASSERT(name != NULL);

	Schema *s = rm_malloc(sizeof(Schema));

	s->id           =  id;
	s->type         =  type;
	s->index        =  NULL;
	s->fulltextIdx  =  NULL;
	s->name         =  rm_strdup(name);

	return s;
}

int Schema_ID
(
	const Schema *s
) {
	ASSERT(s != NULL);

	return s->id;
}

const char *Schema_GetName
(
	const Schema *s
) {
	ASSERT(s);
	return s->name;
}

int Schema_GetID(const Schema *s) {
  ASSERT(s);
  return s->id;
}

bool Schema_HasIndices(const Schema *s) {
	ASSERT(s);
	return (s->fulltextIdx || s->index);
}

unsigned short Schema_IndexCount
(
	const Schema *s
) {
	ASSERT(s != NULL);
	unsigned short n = 0;

	if(s->index) n += 1;
	if(s->fulltextIdx) n += 1;

	return n;
}

Index *Schema_GetIndex
(
	const Schema *s,
	Attribute_ID *attribute_id,
	IndexType type
) {
	ASSERT(s != NULL);

	if(type == IDX_EXACT_MATCH) {
		// return NULL if the index does not exist, or an attribute was
		// specified but does not reside on the index
		if(s->index == NULL ||
		   (attribute_id && !Index_ContainsAttribute(s->index, *attribute_id))) {
			return NULL;
		}
		return s->index;
	} else if(type ==  IDX_FULLTEXT) {
		// return NULL if the index does not exist, or an attribute was
		// specified but does not reside on the index
		if(s->fulltextIdx == NULL ||
		   (attribute_id && !Index_ContainsAttribute(s->fulltextIdx, *attribute_id))) {
			return NULL;
		}
		return s->fulltextIdx;
	} else if(type == IDX_ANY && attribute_id) {
		// if an attribute was specified
		// return the first index that contains it
		if(s->index && Index_ContainsAttribute(s->index, *attribute_id)) {
			return s->index;
		}
		if(s->fulltextIdx && Index_ContainsAttribute(s->fulltextIdx, *attribute_id)) {
			return s->fulltextIdx;
		}
	} else if(type == IDX_ANY && attribute_id == NULL) {
		// if no attribute was specified, return the first extant index
		if(s->index) return s->index;
		if(s->fulltextIdx) return s->fulltextIdx;
	}

	return NULL;
}

int Schema_AddIndex
(
	Index **idx,
	Schema *s,
	IndexField *field,
	IndexType type
) {
	ASSERT(s     != NULL);
	ASSERT(idx   != NULL);
	ASSERT(field != NULL);

	Index *_idx = Schema_GetIndex(s, NULL, type);

	// index exists, make sure attribute isn't already indexed
	if(_idx != NULL) {
		if(Index_ContainsAttribute(_idx, field->id)) {
			IndexField_Free(field);
			return INDEX_FAIL;
		}
	} else {
		// index doesn't exist, create it
		// determine index graph entity type
		GraphEntityType entity_type;
		if(s->type == SCHEMA_NODE) entity_type = GETYPE_NODE;
		else entity_type = GETYPE_EDGE;

		_idx = Index_New(s->name, s->id, type, entity_type);
		if(type == IDX_FULLTEXT) s->fulltextIdx = _idx;
		else s->index = _idx;

		// introduce edge src and dest node ids
		// as additional index fields
		if(entity_type == GETYPE_EDGE) {
			Index_AddField(_idx, INDEX_FIELD_DEFAULT(_src_id));
			Index_AddField(_idx, INDEX_FIELD_DEFAULT(_dest_id));
		}
	}

	Index_AddField(_idx, field);

	*idx = _idx;
	return INDEX_OK;
}

static int _Schema_RemoveExactMatchIndex
(
	Schema *s,
	const char *field
) {
	ASSERT(s != NULL);
	ASSERT(field != NULL);

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Attribute_ID attribute_id = GraphContext_GetAttributeID(gc, field);
	ASSERT(attribute_id != ATTRIBUTE_ID_NONE);

	Index *idx = Schema_GetIndex(s, &attribute_id, IDX_EXACT_MATCH);
	ASSERT(idx != NULL);

	Index_RemoveField(idx, field);

	// if index field count dropped to 0
	// or it is edge index and it dropped to 2(_src_id, _dest_id)
	// remove index from schema
	// index will be freed by the indexer thread
	if(Index_FieldsCount(idx) == 0 ||
	   (s->type == SCHEMA_EDGE && Index_FieldsCount(idx) == 2)) {
		s->index = NULL;
	}

	return INDEX_OK;
}

static int _Schema_RemoveFullTextIndex
(
	Schema *s
) {
	ASSERT(s != NULL);

	GraphContext *gc = QueryCtx_GetGraphCtx();
	Index *idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	ASSERT(idx != NULL);

	// index will be freed by indexer
	s->fulltextIdx = NULL;

	return INDEX_OK;
}

int Schema_RemoveIndex
(
	Schema *s,
	const char *field,
	IndexType type
) {
	ASSERT(s != NULL);

	switch(type) {
		case IDX_FULLTEXT:
			return _Schema_RemoveFullTextIndex(s);
		case IDX_EXACT_MATCH:
			return _Schema_RemoveExactMatchIndex(s, field);
		default:
			return INDEX_FAIL;
	}
}

// index node under all schema indices
void Schema_AddNodeToIndices
(
	const Schema *s,
	const Node *n
) {
	ASSERT(s != NULL);
	ASSERT(n != NULL);

	Index *idx = NULL;

	idx = s->fulltextIdx;
	if(idx) Index_IndexNode(idx, n);

	idx = s->index;
	if(idx) Index_IndexNode(idx, n);
}

// index edge under all schema indices
void Schema_AddEdgeToIndices
(
	const Schema *s,
	const Edge *e
) {
	ASSERT(s != NULL);
	ASSERT(e != NULL);

	Index *idx = NULL;

	idx = s->fulltextIdx;
	if(idx) Index_IndexEdge(idx, e);

	idx = s->index;
	if(idx) Index_IndexEdge(idx, e);
}

// remove node from schema indicies
void Schema_RemoveNodeFromIndices
(
	const Schema *s,
	const Node *n
) {
	ASSERT(s != NULL);
	ASSERT(n != NULL);

	Index *idx = NULL;

	idx = s->fulltextIdx;
	if(idx) Index_RemoveNode(idx, n);

	idx = s->index;
	if(idx) Index_RemoveNode(idx, n);
}

// remove edge from schema indicies
void Schema_RemoveEdgeFromIndices
(
	const Schema *s,
	const Edge *e
) {
	ASSERT(s != NULL);
	ASSERT(e != NULL);

	Index *idx = NULL;

	idx = s->fulltextIdx;
	if(idx) Index_RemoveEdge(idx, e);

	idx = s->index;
	if(idx) Index_RemoveEdge(idx, e);
}

void Schema_Free
(
	Schema *s
) {
	ASSERT(s != NULL);

	if(s->name) rm_free(s->name);

	// Free indicies.
	if(s->index) Index_Free(s->index);
	if(s->fulltextIdx) Index_Free(s->fulltextIdx);

	rm_free(s);
}

