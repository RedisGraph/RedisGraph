/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __SCHEMA_H__
#define __SCHEMA_H__

#include "../redismodule.h"
#include "../index/index.h"
#include "../util/triemap/triemap.h"
#include "../graph/entities/graph_entity.h"

typedef enum {
  SCHEMA_NODE,
  SCHEMA_EDGE,
} SchemaType;

/* Schema represents the structure of a typed graph entity (Node/Edge).
 * similar to a relational table structure, our schemas are a collection
 * of attributes we've encountered overtime as entities were created or updated. */
typedef struct {
  int id;                 /* Internal ID to a matrix within the graph. */
  char *name;             /* Schema name. */
  Index **indices;        /* Indices applicable to schema. */
} Schema;

/* Creates a new schema. */
Schema* Schema_New(const char *label, int id);

/* Given attribute name retrieves its unique ID
 * Return attribute_NOTFOUND if attribute doesn't exists. */
Attribute_ID Attribute_GetID(const char *attribute);

/* Returns number of indices in schema. */
unsigned short Schema_IndexCount(const Schema *s);

/* Retrieves index from attribute. 
 * Returns NULL if index wasn't found. */
Index* Schema_GetIndex(Schema *s, const char* attribute);

/* Assign a new index to attribute
 * attribute must already exists and not associated with an index. */
void Schema_AddIndex(Schema *s, char *attribute, Index *idx);

/* Removes index. */
void Schema_RemoveIndex(Schema *s, const char *attribute);

/* Free schema. */
void Schema_Free(Schema *s);

#endif /* __SCHEMA_H__ */
