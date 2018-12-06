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

/* Get a string representation of record. */
size_t ResultSetRecord_ToString(const ResultSetRecord *record, char **buf, size_t *buf_cap);

/* Frees given record. */
void ResultSetRecord_Free(ResultSetRecord *r);

#endif
