/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "redismodule.h"
#include "util/triemap/triemap.h"
#include "index/index.h"

typedef enum {
  SCHEMA_NODE,
  SCHEMA_EDGE,
} SchemaType; 

// Schema represents the structure of a typed graph entity (Node/Edge).
// Similar to a relational table structure, our schemas are a collection
// of attributes we've encountered overtime as entities were created or updated.
typedef struct {
  int id;                 // Internal ID to a matrix within the graph
  char *name;             // Schema name
  TrieMap *attributes;    // Attributes encountered for schema
  Index** indices;        // Indices applicable to schema
} Schema;

// Creates a new schema
Schema* Schema_New(const char *label, int id);

// Given attribute name retrieves its unique ID
// Return attribute_NOTFOUND if attribute doesn't exists
Attribute_ID Attribute_GetID(SchemaType t, const char *attribute);

// Returns number of attributes in schema
unsigned short Schema_AttributeCount(const Schema *s);

// Given attribute name retrieves its unique ID from given schema
// Return attribute_NOTFOUND if attribute doesn't exists
Attribute_ID Schema_GetAttributeID(Schema *s, const char *attribute);

// Checks to see if schema contains attribute
bool Schema_ContainsAttribute(const Schema *s, const char *attribute);

// Adds a attribute to schema, attribute is added to both
// unified schema and given schema
Attribute_ID Schema_AddAttribute(Schema *s, SchemaType t, const char *attribute);

// Returns number of indices in schema
unsigned short Schema_IndexCount(const Schema *s);

// Retrieves index from attribute
// Returns NULL if index wasn't found
Index* Schema_GetIndex(Schema *s, const char* attribute);

// Assign a new index to attribute
// attribute must already exists and not associated with an index
void Schema_AddIndex(Schema *s, char *attribute, Index *idx);

// Removes index
void Schema_RemoveIndex(Schema *s, const char *attribute);

// Free schema
void Schema_Free(Schema *s);

// Create a map from attribute ID to attribute name
char** Schema_AttributeMap(Schema *s, unsigned short *attr_count);

// Free attribute mapping created by Schema_AttributeMap
void Schema_FreeAttributeMap(char **map, unsigned short map_len);
