/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __RECORD_H_
#define __RECORD_H_

#include "../value.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"

#include <sys/types.h>

typedef enum  {
	REC_TYPE_UNKNOWN = 0,
	REC_TYPE_SCALAR = 1 << 0,
	REC_TYPE_NODE = 1 << 2,
	REC_TYPE_EDGE = 1 << 3,
	REC_TYPE_HEADER = 1 << 4,
} RecordEntryType;

typedef struct {
	union {
		SIValue s;
		Node n;
		Edge e;
	} value;
	RecordEntryType type;
} Entry;

typedef Entry *Record;

// Create a new record capable of holding N entries.
Record Record_New(int entries);

// Extands record to given length.
void Record_Extend(Record *r, int len);

// Clones record, sharing any nested references with the original.
Record Record_Clone(const Record r);

// Extends record to accommodate 'len' entries.
void Record_Extend(Record *r, int len);

// Removes last `count` elements from record.
// TODO: Remove this functions once hash like records are introduced.
void Record_Truncate(Record r, uint count);

// Merge record b into a, sharing any nested references in b with a.
void Record_Merge(Record *a, const Record b);

// Returns number of entries record can hold.
unsigned int Record_length(const Record r);

// Get entry type.
RecordEntryType Record_GetType(const Record r, int idx);

// Get a scalar from record at position idx.
SIValue Record_GetScalar(const Record r, int idx);

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

// Add a scalar to record at position idx.
void Record_AddScalar(Record r, int idx, SIValue v);

// Add a node to record at position idx.
void Record_AddNode(Record r, int idx, Node node);

// Add an edge to record at position idx.
void Record_AddEdge(Record r, int idx, Edge edge);

// String representation of record.
size_t Record_ToString(const Record r, char **buf, size_t *buf_cap);

// 64-bit hash of record
unsigned long long Record_Hash64(const Record r);

// Free record.
void Record_Free(Record r);

#endif
