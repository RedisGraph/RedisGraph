/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/entities/node.h"
#include "../graph/entities/graph_entity.h"
#include "../../deps/RediSearch/src/redisearch_api.h"

typedef struct {
    char *label;                // Indexed label.
    char **fields;              // Indexed fields.
    Attribute_ID *fields_ids;   // Indexed field IDs.
    uint fields_count;          // Number of fields.
    RSIndex *idx;               // RediSearch full-text index.
} FullTextIndex;

// Create a new FullText index.
FullTextIndex* FullTextIndex_New
(
    const char *label   // Indexed label
);

// Adds field to index.
void FullTextIndex_AddField
(
    FullTextIndex *idx,
    const char *field
);

// Removes fields from index.
void FullTextIndex_RemoveField
(
    FullTextIndex *idx,
    const char *field
);

// Index node.
void FullTextIndex_IndexNode
(
    FullTextIndex *idx,     // Index to use
    Node *n                 // Node to index.
);

// Constructs index.
void FullTextIndex_Construct
(
    FullTextIndex *idx
);

// Query index.
RSResultsIterator* FullTextIndex_Query
(
    const FullTextIndex *idx,
    const char *query,          // Query to execute
    char** err                  // Optional, report back error
);

// Return indexed label.
char* FullTextIndex_GetLabel
(
    const FullTextIndex *idx
);

// Returns number of fields indexed.
uint FullTextIndex_FieldsCount
(
    const FullTextIndex *idx
);

// Returns indexed fields.
void FullTextIndex_GetFields
(
    const FullTextIndex *idx,
    char **fields    // Array of size FullTextIndex_FieldsCount
);

// Checks if given field is indexed.
bool FullTextIndex_ContainsField
(
    const FullTextIndex *idx,
    const char *field
);

// Free fulltext index.
void FullTextIndex_Free
(
    FullTextIndex *idx
);
