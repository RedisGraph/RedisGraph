/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "resultset_record.h"
#include "../query_executor.h"

ResultSetRecord* NewResultSetRecord(size_t len) {
    ResultSetRecord *r = (ResultSetRecord*)malloc(sizeof(ResultSetRecord));
    r->len = len;
    r->values = malloc(sizeof(SIValue) * len);
    return r;
}

size_t ResultSetRecord_ToString(const ResultSetRecord *record, char **buf, size_t *buf_cap) {
    size_t required_len = SIValue_StringConcatLen(record->values, record->len);

    if(*buf_cap < required_len) {
        *buf = realloc(*buf, sizeof(char) * required_len);
        *buf_cap = required_len;
    }

    return SIValue_StringConcat(record->values, record->len, *buf, *buf_cap);
}

/* Frees given record. */
void ResultSetRecord_Free(ResultSetRecord *r) {
    if(r == NULL) return;
    for (int i = 0; i < r->len; i ++) {
        SIValue_Free(&r->values[i]);
    }
    free(r->values);
    free(r);
}
