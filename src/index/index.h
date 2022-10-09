/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include "../graph/entities/graph_entity.h"
#include "../graph/graph.h"
#include "redisearch_api.h"

#include <stdatomic.h>

#define INDEX_OK 1
#define INDEX_FAIL 0
#define INDEX_SEPARATOR '\1'  // can't use '\0', RediSearch will terminate on \0
#define INDEX_FIELD_NONE_INDEXED "NONE_INDEXABLE_FIELDS"

#define INDEX_FIELD_DEFAULT_WEIGHT 1.0
#define INDEX_FIELD_DEFAULT_NOSTEM false
#define INDEX_FIELD_DEFAULT_PHONETIC "no"

// creates a new index field and initialize it to default values
// returns a pointer to the field
#define INDEX_FIELD_DEFAULT(field)                                                    \
	({                                                                                \
		IndexField field;                                                             \
		IndexField_New(&field, ATTRIBUTE_ID_NONE, #field, INDEX_FIELD_DEFAULT_WEIGHT, \
			INDEX_FIELD_DEFAULT_NOSTEM, INDEX_FIELD_DEFAULT_PHONETIC);                \
		&field;                                                                       \
	})

typedef enum {
	IDX_ANY          =  0,
	IDX_EXACT_MATCH  =  1,
	IDX_FULLTEXT     =  2,
} IndexType;

typedef struct {
	EntityID src_id;
	EntityID dest_id;
	EntityID edge_id;
} EdgeIndexKey;

typedef struct {
	char *name;        // field name
	Attribute_ID id;   // field id
	double weight;     // the importance of text
	bool nostem;       // disable stemming of the text
	char *phonetic;    // phonetic search of text
} IndexField;

typedef struct {
	char *label;                   // indexed label
	int label_id;                  // indexed label ID
	IndexField *fields;            // indexed fields
	char *language;                // language
	char **stopwords;              // stopwords
	GraphEntityType entity_type;   // entity type (node/edge) indexed
	IndexType type;                // index type exact-match / fulltext
	RSIndex *idx;                  // rediSearch index
	uint _Atomic pending_changes;  // number of pending changes
} Index;

// create new index field
void IndexField_New
(
	IndexField *field,    // field to initialize
	Attribute_ID id,      // field id
	const char *name,     // field name
	double weight,        // the importance of text
	bool nostem,          // disable stemming of the text
	const char *phonetic  // phonetic search of text
);

// free index field
void IndexField_Free
(
	IndexField *field  // index field to be freed
);

// create a new index
Index *Index_New
(
	const char *label,           // indexed label
	int label_id,                // indexed label id
	IndexType type,              // exact match or full text
	GraphEntityType entity_type  // entity type been indexed
);

// disable index by increasing the number of pending changes
// and re-creating the internal RediSearch index
void Index_Disable
(
	Index *idx  // index to disable
);

// returns true if index state is IDX_OPERATIONAL
bool Index_Enabled
(
	const Index *idx  // index to get state of
);

// populates index
void Index_Populate
(
	Index *idx,  // index to populate
	Graph *g     // graph holding entities to index
);

// adds field to index
void Index_AddField
(
	Index *idx,        // index modified
	IndexField *field  // field added
);

// removes field from index
void Index_RemoveField
(
	Index *idx,        // index modified
	const char *field  // field to remove
);

// index node
void Index_IndexNode
(
	Index *idx,    // index to populate
	const Node *n  // node to index
);

// index edge
void Index_IndexEdge
(
	Index *idx,    // index to populate
	const Edge *e  // edge to index
);

// remove node from index
void Index_RemoveNode
(
	Index *idx,    // index to update
	const Node *n  // node to remove from index
);

// remove edge from index
void Index_RemoveEdge
(
	Index *idx,    // index to update
	const Edge *e  // edge to remove from index
);

// query an index
RSResultsIterator *Index_Query
(
	const Index *idx,   // index to query
	const char *query,  // query to execute
	char **err          // [optional] report back error
);

// returns number of fields indexed
uint Index_FieldsCount
(
	const Index *idx  // index to query
);

// returns a shallow copy of indexed fields
const IndexField *Index_GetFields
(
	const Index *idx  // index to query
);

// checks if given attribute is indexed
bool Index_ContainsAttribute
(
	const Index *idx,          // index to query
	Attribute_ID attribute_id  // attribute id to search
);

// returns indexed label ID
int Index_GetLabelID
(
	const Index *idx  // index to query
);

// returns indexed language
const char *Index_GetLanguage
(
	const Index *idx  // index to query
);

// returns indexed stopwords
char **Index_GetStopwords
(
	const Index *idx,  // index to query
	size_t *size       // number of stopwords
);

// set indexed language
void Index_SetLanguage
(
	Index *idx,           // index modified
	const char *language  // language to set
);

// set indexed stopwords
void Index_SetStopwords
(
	Index *idx,       // index modified
	char **stopwords  // stopwords
);

// free fulltext index
void Index_Free
(
	Index *idx  // index being freed
);

