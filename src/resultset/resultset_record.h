/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __RESULTSET_RECORD_H__
#define __RESULTSET_RECORD_H__

#include "resultset_header.h"
#include "../parser/ast.h"
#include "../grouping/group.h"

typedef struct {
    unsigned int len;
    SIValue *values;
} ResultSetRecord;

/* Creates a new record which will hold len elements. */
ResultSetRecord* NewResultSetRecord(size_t len);

/* Creates a new record from an aggregated group. */
ResultSetRecord* ResultSetRecord_FromGroup(const ResultSetHeader *resultset_header, const Group *g);

/* Get a string representation of record. */
size_t ResultSetRecord_ToString(const ResultSetRecord *record, char **buf, size_t *buf_cap);

/* Compares the two records, using the values at given indices
 * Returns 1 if A >= B, -1 if A <= B, 0 if A = B */
int ResultSetRecord_Compare(const ResultSetRecord *A, const ResultSetRecord *B, int *compareIndices, size_t compareIndicesLen);

/* Frees given record. */
void ResultSetRecord_Free(ResultSetRecord *r);

#endif
