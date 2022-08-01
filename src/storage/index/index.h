/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../entities/node.h"
#include "../entities/edge.h"
#include "../entities/graph_entity.h"
#include "../graph/graph.h"
#include "redisearch_api.h"

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
	char *label;                  // indexed label
	int label_id;                 // indexed label ID
	IndexField *fields;           // indexed fields
	char *language;               // language
	char **stopwords;             // stopwords
	GraphEntityType entity_type;  // entity type (node/edge) indexed
	IndexType type;               // index type exact-match / fulltext
	RSIndex *idx;                 // rediSearch index
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

void IndexField_Free
(
	IndexField *field
);

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
	Index *idx,
	Graph *g
);

// adds field to index
void Index_AddField
(
	Index *idx,
	IndexField *field
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
const IndexField *Index_GetFields
(
	const Index *idx
);

// checks if given attribute is indexed
bool Index_ContainsAttribute
(
	const Index *idx,
	Attribute_ID attribute_id  // attribute id to search
);

// returns indexed label ID
int Index_GetLabelID
(
	const Index *idx
);

// returns indexed language
const char *Index_GetLanguage
(
	const Index *idx
);

// returns indexed stopwords
char **Index_GetStopwords
(
	const Index *idx,
	size_t *size
);

// set indexed language
void Index_SetLanguage
(
	Index *idx,
	const char *language
);

// set indexed stopwords
void Index_SetStopwords
(
	Index *idx,
	char **stopwords
);

// free fulltext index
void Index_Free
(
	Index *idx
);
