/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "schema.h"
#include "../errors.h"
#include "../util/arr.h"
#include "../query_ctx.h"
#include "../util/rmalloc.h"
#include "../graph/graphcontext.h"
#include "../constraint/constraint.h"

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
	s->constraints  =  array_new(Constraint, 0);
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

// return schema type
SchemaType Schema_GetType
(
	const Schema *s
) {
	ASSERT(s != NULL);
	return s->type;
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

// get index from schema
// returns NULL if index wasn't found
Index Schema_GetIndex
(
	const Schema *s,            // schema to get index from
	const Attribute_ID *attrs,  // indexed attributes
	uint n,                     // number of attributes
	IndexType type              // type of index
) {
	// validations
	ASSERT(s != NULL);
	ASSERT((attrs == NULL && n == 0) || (attrs != NULL && n > 0));

	Index idx         = NULL;  // index to return
	uint  idx_count   = 0;     // number of indicies to consider
	Index indicies[2] = {0};   // indicies to consider

	if(type != IDX_ANY) {
		// consider specified index
		idx_count   = 1;
		indicies[0] = (type == IDX_EXACT_MATCH) ? s->index : s->fulltextIdx;
	} else  {
		// consider both exact-match and fulltext indicies
		idx_count   = 2;
		indicies[0] = s->index;
		indicies[1] = s->fulltextIdx;
	}

	//--------------------------------------------------------------------------
	// return first index which contains all specified attributes
	//--------------------------------------------------------------------------

	for(uint i = 0; i < idx_count; i++) {
		idx = indicies[i];

		// index doesn't exists
		if(idx == NULL) {
			continue;
		}

		// make sure index contains all specified attributes
		bool all_attr_found = true;
		for(uint i = 0; i < n; i++) {
			if(!Index_ContainsAttribute(idx, attrs[i])) {
				idx = NULL;
				all_attr_found = false;
				break;
			}
		}

		if(all_attr_found == true) {
			break;
		}
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
	Index _idx = Schema_GetIndex(s, NULL, 0, type);

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

	// convert attribute name to attribute ID
	Attribute_ID attr_id = GraphContext_GetAttributeID(gc, field);
	if(attr_id == ATTRIBUTE_ID_NONE) {
		return INDEX_FAIL;
	}

	// try to get index
	Index idx = Schema_GetIndex(s, &attr_id, 1, IDX_EXACT_MATCH);
	if(idx == NULL) {
		return INDEX_FAIL;
	}

	//--------------------------------------------------------------------------
	// make sure index doesn't supports any constraints
	//--------------------------------------------------------------------------

	uint n = array_len(s->constraints);
	for(uint i = 0; i < n; i++) {
		Constraint c = s->constraints[i];
		if(Constraint_GetStatus(c) != CT_FAILED &&
		   Constraint_GetType(c) == CT_UNIQUE   &&
		   Constraint_ContainsAttribute(c, attr_id)) {
			ErrorCtx_SetError("Index supports constraint");
			return INDEX_FAIL;
		}
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

	Index idx = Schema_GetIndex(s, NULL, 0, IDX_FULLTEXT);
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

//------------------------------------------------------------------------------
// constraints API
//------------------------------------------------------------------------------

// check if schema has constraints
bool Schema_HasConstraints
(
	const Schema *s  // schema to query
) {
	ASSERT(s != NULL);
	return (s->constraints != NULL && array_len(s->constraints) > 0);
}

// checks if schema constains constraint
bool Schema_ContainsConstraint
(
	const Schema *s,            // schema to search
	ConstraintType t,           // constraint type
	const Attribute_ID *attrs,  // constraint attributes
	uint attr_count             // number of attributes
) {
	// validations
	ASSERT(s          != NULL);
	ASSERT(attrs      != NULL);
	ASSERT(attr_count > 0);

	Constraint c = Schema_GetConstraint(s, t, attrs, attr_count);
	return (c != NULL && Constraint_GetStatus(c) != CT_FAILED);
}

// retrieves constraint 
// returns NULL if constraint was not found
Constraint Schema_GetConstraint
(
	const Schema *s,            // schema from which to get constraint
	ConstraintType t,           // constraint type
	const Attribute_ID *attrs,  // constraint attributes
	uint attr_count             // number of attributes
) {
	// validations
	ASSERT(s          != NULL);
	ASSERT(attrs      != NULL);
	ASSERT(attr_count > 0);

	// search for constraint
	uint n = array_len(s->constraints);
	for(uint i = 0; i < n; i++) {
		Constraint c = s->constraints[i];

		// check constraint type
		if(Constraint_GetType(c) != t) {
			continue;
		}

		// make sure constraint attribute count matches
		const Attribute_ID *c_attrs;
		uint n = Constraint_GetAttributes(c, &c_attrs, NULL);
		if(n != attr_count) {
			continue;
		}

		// match each attribute
		bool match = true;  // optimistic
		for(uint j = 0; j < n; j++) {
			if(c_attrs[j] != attrs[j]) {
				match = false;
				break;
			}
		}

		if(match == true) {
			return c;
		}
	}

	// no constraint was found
	return NULL;
}

// get all constraints in schema
const Constraint *Schema_GetConstraints
(
	const Schema *s  // schema from which to extract constraints
) {
	// validations
	ASSERT(s != NULL);
	ASSERT(s->constraints != NULL);

	return s->constraints;
}

// adds a constraint to schema
void Schema_AddConstraint
(
	Schema *s,       // schema holding the index
	Constraint c     // constraint to add
) {
	ASSERT(s != NULL);
	ASSERT(c != NULL);
	array_append(s->constraints, c);
}

// removes constraint from schema
void Schema_RemoveConstraint
(
	Schema *s,    // schema
	Constraint c  // constraint to remove
) {
	// validations
	ASSERT(s != NULL);
	ASSERT(c != NULL);

	// search for constraint
	uint n = array_len(s->constraints);
	for(uint i = 0; i < n; i++) {
		if(c == s->constraints[i]) {
			Constraint_IncPendingChanges(c);
			array_del_fast(s->constraints, i);
			return;
		}
	}

	ASSERT(false);
}

// enforce all constraints under given schema on entity
bool Schema_EnforceConstraints
(
	const Schema *s,       // schema
	const GraphEntity *e,  // entity to enforce
	char **err_msg         // report error message
) {
	// validations
	ASSERT(s != NULL);
	ASSERT(e != NULL);
	
	// see if entity holds under all schema's constraints
	uint n = array_len(s->constraints);
	for(uint i = 0; i < n; i++) {
		Constraint c = s->constraints[i];
		if(Constraint_GetStatus(c) != CT_FAILED &&
		   !Constraint_EnforceEntity(c, e, err_msg)) {
			// entity failed to pass constraint
			return false;
		}
	}

	// entity passed all constraint
	return true;
}

void Schema_Free
(
	Schema *s
) {
	ASSERT(s != NULL);

	if(s->name) {
		rm_free(s->name);
	}

	// free constraints
	if(s->constraints != NULL) {
		uint n = array_len(s->constraints);
		for(uint i = 0; i < n; i++) {
			Constraint_Free(s->constraints + i);
		}
		array_free(s->constraints);
	}

	// free indicies
	if(s->index != NULL) {
		Index_Free(s->index);
	}

	if(s->fulltextIdx != NULL) {
		Index_Free(s->fulltextIdx);
	}

	rm_free(s);
}

