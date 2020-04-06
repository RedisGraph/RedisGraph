/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../value.h"
#include "../../deps/rax/rax.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"
#include <sys/types.h>

// Return value in case of call to Record_GetEntryIdx with invalid entry alias.
#define INVALID_INDEX -1

typedef enum  {
	REC_TYPE_UNKNOWN = 0,
	REC_TYPE_SCALAR = 1 << 0,
	REC_TYPE_NODE = 1 << 1,
	REC_TYPE_EDGE = 1 << 2,
	REC_TYPE_HEADER = 1 << 3,
} RecordEntryType;

typedef struct {
	union {
		SIValue s;
		Node n;
		Edge e;
	} value;
	RecordEntryType type;
} Entry;

typedef struct {
	void *owner;        // Owner of record.
	rax *mapping;       // Mapping between alias to record entry.
	Entry entries[];    // Array of entries.
} _Record;

typedef _Record *Record;

// Create a new record sized to accommodate all entries in the given map.
Record Record_New(rax *mapping);

// Clones record.
void Record_Clone(const Record r, Record clone);

// Merge record b into a, sharing any nested references in b with a.
void Record_Merge(Record *a, const Record b);

// Merge record b into a, transfer value ownership from b to a.
void Record_TransferEntries(Record *to, Record from);

// Returns number of entries record can hold.
uint Record_length(const Record r);

// Return alias position within the record.
int Record_GetEntryIdx(Record r, const char *alias);

// Get entry type.
RecordEntryType Record_GetType(const Record r, int idx);

// Get a node from record at position idx.
Node *Record_GetNode(const Record r, int idx);

// Get an edge from record at position idx.
Edge *Record_GetEdge(const Record r, int idx);

// Get an SIValue containing the entity at position idx.
SIValue Record_Get(Record r, int idx);

// Get a graph entity from record at position idx.
GraphEntity *Record_GetGraphEntity(const Record r, int idx);

// Add a scalar, node, or edge to the record, depending on the SIValue type.
void Record_Add(Record r, int idx, SIValue v);

// Add a scalar to record at position idx and return a reference to it.
SIValue *Record_AddScalar(Record r, int idx, SIValue v);

// Add a node to record at position idx and return a reference to it.
Node *Record_AddNode(Record r, int idx, Node node);

// Add an edge to record at position idx and return a reference to it.
Edge *Record_AddEdge(Record r, int idx, Edge edge);

// Ensure that all scalar values in record are access-safe.
void Record_PersistScalars(Record r);

// String representation of record.
size_t Record_ToString(const Record r, char **buf, size_t *buf_cap);

// 64-bit hash of record
unsigned long long Record_Hash64(const Record r);

// Free record entries.
void Record_FreeEntries(Record r);

// Free record.
void Record_Free(Record r);

