#ifndef __GRAPH_RECORD_H__
#define __GRAPH_RECORD_H__

#include "resultset_header.h"
#include "../parser/ast.h"
#include "../grouping/group.h"
#include "../graph/graph.h"

typedef struct {
    unsigned int len;
    SIValue *values;
} Record;

/* Creates a new record which will hold len elements. */
Record* NewRecord(size_t len);

/* Creates a new record from an aggregated group. */
Record* Record_FromGroup(const ResultSetHeader *resultset_header, const Group *g);

/* Get a string representation of record. */
size_t Record_ToString(const Record *record, char **record_str);

/* Compares the two records, using the values at given indeces
 * Returns 1 if A >= B, -1 if A <= B, 0 if A = B */
int Records_Compare(const Record *A, const Record *B, int *compareIndices, size_t compareIndicesLen);

/* Frees given record. */
void Record_Free(Record *r);

#endif