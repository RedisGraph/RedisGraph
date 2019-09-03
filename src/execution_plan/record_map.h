/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../ast/ast.h"
#include <limits.h>
#include <stddef.h>
#include "rax.h"

#define IDENTIFIER_NOT_FOUND UINT_MAX

typedef struct {
	rax *map;       // Mapping of AST IDs and aliases to Record IDs
	uint record_len;    // Length of Record being modified by this segment.
} RecordMap;

/* Retrieve a Record ID given an alias, returning IDENTIFER_NOT_FOUND if alias is not mapped. */
uint RecordMap_LookupAlias(const RecordMap *map, const char *alias);

/* Retrieve a Record ID given an AST/QueryGraph ID, returning IDENTIFER_NOT_FOUND if ID is not mapped. */
uint RecordMap_LookupID(const RecordMap *record_map, uint id);

/* Retrieve a Record ID given an alias, creating a new ID if not previously mapped. */
uint RecordMap_FindOrAddAlias(RecordMap *record_map, const char *alias);

/* Retrieve a Record ID given an AST/QueryGraph ID, creating a new ID if not previously mapped. */
uint RecordMap_FindOrAddID(RecordMap *record_map, uint entity_id);

/* Create a new RecordMap. */
RecordMap *RecordMap_New(void);

/* Free the given RecordMap. */
void RecordMap_Free(RecordMap *record_map);
