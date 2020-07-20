/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/node.h"
#include "../graph/entities/graph_entity.h"
#include "redisearch_api.h"

#define INDEX_OK 1
#define INDEX_FAIL 0

typedef enum {
	IDX_ANY,
	IDX_EXACT_MATCH,
	IDX_FULLTEXT,
} IndexType;

typedef struct {
	char *label;                // Indexed label.
	char **fields;              // Indexed fields.
	Attribute_ID *fields_ids;   // Indexed field IDs.
	uint fields_count;          // Number of fields.
	RSIndex *idx;               // RediSearch index.
	IndexType type;             // Index type exact-match / fulltext.
} Index;

// Create a new FullText index.
Index *Index_New
(
	const char *label,  // Indexed label
	IndexType type      // Index is a fulltext index
);

// Adds field to index.
void Index_AddField
(
	Index *idx,
	const char *field
);

// Removes fields from index.
void Index_RemoveField
(
	Index *idx,
	const char *field
);

// Index node.
void Index_IndexNode
(
	Index *idx,     // Index to use
	const Node *n   // Node to index
);

// Remove node from index.
void Index_RemoveNode
(
	Index *idx,     // Index to use
	const Node *n   // Node to remove
);

// Constructs index.
void Index_Construct
(
	Index *idx
);

// Query index.
RSResultsIterator *Index_Query
(
	const Index *idx,
	const char *query,          // Query to execute
	char **err                  // Optional, report back error
);

// Return indexed label.
const char *Index_GetLabel
(
	const Index *idx
);

// Returns number of fields indexed.
uint Index_FieldsCount
(
	const Index *idx
);

// Returns indexed fields.
const char **Index_GetFields
(
	const Index *idx
);

// Checks if given field is indexed.
bool Index_ContainsField
(
	const Index *idx,
	const char *field
);

// Free fulltext index.
void Index_Free
(
	Index *idx
);
