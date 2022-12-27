/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "../redismodule.h"
#include "../index/index.h"
#include "rax.h"
#include "redisearch_api.h"
#include "../graph/entities/graph_entity.h"
#include "../constraint/constraint.h"

typedef enum {
	SCHEMA_NODE,
	SCHEMA_EDGE,
} SchemaType;

// schema represents the structure of a typed graph entity (Node/Edge)
// similar to a relational table structure, our schemas are a collection
// of attributes we've encountered overtime as entities were created or updated
typedef struct {
	int id;                 // schema id
	char *name;             // schema name
	SchemaType type;        // schema type (node/edge)
	Index index;            // exact match index
	Index fulltextIdx;      // full-text index
	Constraint constraints; // constraints array
} Schema;

// creates a new schema
Schema *Schema_New
(
	SchemaType type,
	int id,
	const char *name
);

/* Return the given schema's name. */
const char *Schema_GetName(const Schema *s);

/* Return the given schema's ID. */
int Schema_GetID(const Schema *s);

const char *Schema_GetName
(
	const Schema *s
);

// returns true if schema has either a full-text or exact-match index
bool Schema_HasIndices
(
	const Schema *s
);

// returns number of indices in schema
unsigned short Schema_IndexCount
(
	const Schema *s
);

// retrieves index from attribute
// returns NULL if index wasn't found
Index Schema_GetIndex
(
	const Schema *s,
	Attribute_ID *attribute_id,
	IndexType type
);

// assign a new index to attribute
// attribute must already exists and not associated with an index
int Schema_AddIndex
(
	Index *idx,         // [input/output] index to create
	Schema *s,          // schema holding the index
	IndexField *field,  // field to index
	IndexType type,      // type of entities to index
	bool inc_ref_count  // should the index's ref count be incremented?
);

// removes index
int Schema_RemoveIndex
(
	Schema *s,
	const char *field,
	IndexType type,
	bool part_of_constraint_deletion
);

// introduce node to schema indicies
void Schema_AddNodeToIndices
(
	const Schema *s,
	const Node *n
);

// introduce edge to schema indicies
void Schema_AddEdgeToIndices
(
	const Schema *s,
	const Edge *e
);

// remove node from schema indicies
void Schema_RemoveNodeFromIndices
(
	const Schema *s,
	const Node *n
);

// remove edge from schema indicies
void Schema_RemoveEdgeFromIndices
(
	const Schema *s,
	const Edge *e
);

// Free schema
void Schema_Free
(
	Schema *s
);

bool Schema_HasConstraints(const Schema *s);

// checks if schema has a constraint
bool Schema_ContainsConstraint(
const Schema *s, 
const ConstAttrData *fields, 
uint field_count);

// adds a constraint to schema
int Schema_AddConstraint
(
	Schema *s,       // schema holding the index
	Constraint c     // constraint to add
);

Constraint Schema_GetConstraints
(
	const Schema *s,       // schema holding the index
);
