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
#define INDEX_SEPARATOR '\1'  // can't use '\0', RediSearch will terminate on \0
#define INDEX_FIELD_NONE_INDEXED "NONE_INDEXABLE_FIELDS"

typedef enum {
	IDX_ANY = 0,
	IDX_EXACT_MATCH = 1,
	IDX_FULLTEXT = 2,
} IndexType;

typedef struct {
	char *label;                // Indexed label.
	char **fields;              // Indexed fields.
	char **stopwords;           // Stopwords.
	char *language;             // Language.
	Attribute_ID *fields_ids;   // Indexed field IDs.
	RSIndex *idx;               // RediSearch index.
	IndexType type;             // Index type exact-match / fulltext.
} Index;

/**
 * @brief  Create a new index.
 * @param  *label: Indexed label
 * @param  type: Index type - exact match or full text.
 * @retval New constructed index for the label.
 */
Index *Index_New(const char *label, IndexType type);

/**
 * @brief  Adds field to index.
 * @param  *idx: Index
 * @param  *field: Field to add.
 */
void Index_AddField(Index *idx, const char *field);

/**
 * @brief  Removes field from index.
 * @param  *idx: Index
 * @param  *field: Field to remove.
 */
void Index_RemoveField(Index *idx, const char *field);

/**
 * @brief  Index node.
 * @param  *idx: Index
 * @param  *n :Node
 */
void Index_IndexNode(Index *idx, const Node *n);

/**
 * @brief  Remove node from index.
 * @param  *idx: Index to remove the node from.
 * @param  *n: Node to remove.
 */
void Index_RemoveNode(Index *idx, const Node *n);

/**
 * @brief  Constructs index.
 * @param  *idx:
 */
void Index_Construct(Index *idx);

/**
 * @brief  Query an index.
 * @param  *idx: Index.
 * @param  *query: Query to execute.
 * @param  **err: Optional, report back error
 * @retval RedisSearch results iterator.
 */
RSResultsIterator *Index_Query(const Index *idx, const char *query, char **err);

/**
 * @brief Return indexed label.
 * @param  *idx: Index.
 * @retval Index's label.
 */
const char *Index_GetLabel(const Index *idx);

/**
 * @brief  Returns number of fields indexed.
 * @param  *idx: Index.
 * @retval Number of indexed fields.
 */
uint Index_FieldsCount(const Index *idx);

/**
 * @brief  Returns indexed fields.
 * @note   Returns a shallow copy.
 * @param  *idx: Index to extract fields from.
 * @retval Array with the indexed fields.
 */
const char **Index_GetFields(const Index *idx);

/**
 * @brief  Checks if given attribute is indexed.
 * @param  *idx: Index to perform the check.
 * @param  attribute_id: Attribute id to search.
 * @retval True if the attribute is indexed.
 */
bool Index_ContainsAttribute(const Index *idx, Attribute_ID attribute_id);

/**
 * @brief  Free fulltext index.
 * @param  *idx: Index to drop.
 */
void Index_Free(Index *idx);
