/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../graph/entities/graph_entity.h"
#include "redisearch_api.h"

#define INDEX_OK 1
#define INDEX_FAIL 0
#define INDEX_SEPARATOR '\1'  // can't use '\0', RediSearch will terminate on \0
#define INDEX_FIELD_NONE_INDEXED "NONE_INDEXABLE_FIELDS"

typedef enum {
	IDX_ANY = 0,
	IDX_EXACT_MATCH = 1,
	IDX_FULLTEXT = 2,
} IndexType;

typedef struct {
	char *label;                  // indexed label
	int label_id;                 // indexed label ID
	char **fields;                // indexed fields
	Attribute_ID *fields_ids;     // indexed field IDs
	uint fields_count;            // number of fields
	RSIndex *idx;                 // rediSearch index
	IndexType type;               // index type exact-match / fulltext
	GraphEntityType entity_type;  // entity type (node/edge) indexed
} Index;


// create a new index
Index *Index_New
(
	const char *label,           // indexed label
	int label_id,                // indexed label id
	IndexType type,              // exact match or full text
	GraphEntityType entity_type  // entity type been indexed
);

// constructs index
void Index_Construct
(
	Index *idx
);

// adds field to index
void Index_AddField
(
	Index *idx,
	const char *field  // field to add
);

// removes field from index
void Index_RemoveField
(
	Index *idx,
	const char *field  // field to remove
);

// index node
void Index_IndexNode
(
	Index *idx,
	const Node *n  // node to index
);

// index edge
void Index_IndexEdge
(
	Index *idx,
	const Edge *e  // edge to index
);

void Index_RemoveNode
(
	Index *idx,    // index to update
	const Node *n  // node to remove from index
);

void Index_RemoveEdge
(
	Index *idx,    // index to update
	const Edge *e  // edge to remove from index
);

// query an index
RSResultsIterator *Index_Query
(
	const Index *idx,
	const char *query,  // query to execute
	char **err          // [optional] report back error
);

// returns number of fields indexed
uint Index_FieldsCount
(
	const Index *idx
);

// returns a shallow copy of indexed fields
const char **Index_GetFields
(
	const Index *idx
);

// checks if given attribute is indexed
bool Index_ContainsAttribute
(
	const Index *idx,
	Attribute_ID attribute_id  // attribute id to search
);

// free fulltext index
void Index_Free
(
	Index *idx
);

