/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./record.h"
#include "../util/rmalloc.h"
#include <assert.h>

Record Record_New(int entries) {
    Record r = rm_calloc(entries, sizeof(Entry));
    return r;
}

RecordEntryType Record_GetType(const Record r, int idx) {
    return r[idx].type;
}

SIValue Record_GetScalar(Record r,  int idx) {
    r[idx].type = REC_TYPE_SCALAR;
    return r[idx].value.s;
}

Node *Record_GetNode(const Record r,  int idx) {
    r[idx].type = REC_TYPE_NODE;
    return &r[idx].value.n;
}

Edge *Record_GetEdge(const Record r,  int idx) {
    r[idx].type = REC_TYPE_EDGE;
    return &r[idx].value.e;
}

GraphEntity *Record_GetGraphEntity(const Record r, int idx) {
    Entry e = r[idx];
    switch(e.type) {
        case REC_TYPE_NODE:
            return (GraphEntity*)Record_GetNode(r, idx);
        case REC_TYPE_EDGE:
            return (GraphEntity*)Record_GetEdge(r, idx);
        case REC_TYPE_SCALAR:
            return (GraphEntity*)(Record_GetScalar(r, idx).ptrval);
        default:
            assert(false);
    }
    return NULL;
}

void Record_AddScalar(Record r, int idx, SIValue v) {
    r[idx].value.s = v;
    r[idx].type = REC_TYPE_SCALAR;
}

void Record_AddNode(Record r, int idx, Node node) {
    r[idx].value.n = node;
    r[idx].type = REC_TYPE_NODE;
}

void Record_AddEdge(Record r, int idx, Edge edge) {
    r[idx].value.e = edge;
    r[idx].type = REC_TYPE_EDGE;
}

void Record_Free(Record r) {
    rm_free(r);
}
