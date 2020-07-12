/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../redismodule.h"
#include "../index/index.h"
#include "rax.h"
#include "redisearch_api.h"
#include "../graph/entities/graph_entity.h"

typedef enum {
	SCHEMA_NODE,
	SCHEMA_EDGE,
} SchemaType;

/* Schema represents the structure of a typed graph entity (Node/Edge).
 * similar to a relational table structure, our schemas are a collection
 * of attributes we've encountered overtime as entities were created or updated. */
typedef struct {
	int id;               // Internal ID to a matrix within the graph.
	char *name;           // Schema name.
	Index *index;         // Exact match index.
	Index *fulltextIdx;   // Full-text index.
} Schema;

/* Creates a new schema. */
Schema *Schema_New(const char *label, int id);

const char *Schema_GetName(const Schema *s);

/* Returns true if schema has either a full-text or exact-match index. */
bool Schema_HasIndices(const Schema *s);

/* Returns number of indices in schema. */
unsigned short Schema_IndexCount(const Schema *s);

/* Retrieves index from attribute.
 * Returns NULL if index wasn't found. */
Index *Schema_GetIndex(const Schema *s, const char *field, IndexType type);

/* Assign a new index to attribute
 * attribute must already exists and not associated with an index. */
int Schema_AddIndex(Index **idx, Schema *s, const char *field, IndexType type);

/* Removes index. */
int Schema_RemoveIndex(Schema *s, const char *field, IndexType type);

/* Introduce node schema indicies */
void Schema_AddNodeToIndices(const Schema *s, const Node *n);

/* Free schema. */
void Schema_Free(Schema *s);

