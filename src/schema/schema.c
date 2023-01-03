/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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

Index Schema_GetIndex
(
	const Schema *s,
	Attribute_ID *attr_id,
	IndexType type
) {
	ASSERT(s != NULL);

	Index idx = NULL;
	if(type != IDX_ANY) {
		idx = (type == IDX_EXACT_MATCH) ? s->index : s->fulltextIdx;
		// return NULL if the index does not exist, or an attribute was
		// specified but does not reside on the index
		if(idx != NULL && (attr_id && !Index_ContainsAttribute(idx, *attr_id))) {
			idx = NULL;
		}
	} else if(attr_id) {
		// ANY index, specified attribute id
		// return the first index containing attribute
		if(s->index && Index_ContainsAttribute(s->index, *attr_id)) {
			idx = s->index;
		}
		if(s->fulltextIdx && Index_ContainsAttribute(s->fulltextIdx, *attr_id)) {
			idx = s->fulltextIdx;
		}
	} else {
		// ANY index, unspecified attribute id, return the first extant index
		idx = (s->index != NULL) ? s->index : s->fulltextIdx;
	}

	return idx;
}

// assign a new index to attribute
// attribute must already exists and not associated with an index
int Schema_AddIndex
(
	Index *idx,         // [input/output] index to create
	Schema *s,          // schema holding the index
	IndexField *field,  // field to index
	IndexType type      // type of entities to index
) {
	ASSERT(s     != NULL);
	ASSERT(idx   != NULL);
	ASSERT(field != NULL);

	// see if index already exists
	Index _idx = Schema_GetIndex(s, NULL, type);

	// index exists, make sure attribute isn't already indexed
	if(_idx != NULL) {
		if(Index_ContainsAttribute(_idx, field->id)) {
			// field already indexed, quick return
			IndexField_Free(field);
			return INDEX_FAIL;
		}
	} else {
		// index doesn't exist, create it
		// determine index graph entity type
		GraphEntityType entity_type;
		entity_type = (s->type == SCHEMA_NODE) ? GETYPE_NODE : GETYPE_EDGE;

		_idx = Index_New(s->name, s->id, type, entity_type);

		if(type == IDX_FULLTEXT) {
			s->fulltextIdx = _idx;
		} else {
			s->index = _idx;
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
	if(attribute_id == ATTRIBUTE_ID_NONE) {
		return INDEX_FAIL;
	}

	Index idx = Schema_GetIndex(s, &attribute_id, IDX_EXACT_MATCH);
	if(idx == NULL) {
		return INDEX_FAIL;
	}

	Index_RemoveField(idx, field);

	// if index field count dropped to 0 remove index from schema
	// index will be freed by the indexer thread
	if(Index_FieldsCount(idx) == 0) {
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
	Index idx = Schema_GetIndex(s, NULL, IDX_FULLTEXT);
	if(idx == NULL) {
		return INDEX_FAIL;
	}

	Index_Disable(idx);

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

	Index idx = NULL;

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

	Index idx = NULL;

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

	Index idx = NULL;

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

	Index idx = NULL;

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

