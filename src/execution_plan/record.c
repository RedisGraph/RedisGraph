/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./record.h"
#include "../util/uthash.h"
#include <assert.h>

Record Record_Empty() {
    return NULL;
}

void Record_AddEntry(Record *r, const char* k, SIValue v) {
    Entry *e;
    HASH_FIND_STR(*r, k, e);
    if(e) {
         // key already exists, update entry.
         e->v = v;
    } else {
        e = malloc(sizeof(Entry));
        e->alias = k;
        e->v = v;
        HASH_ADD_KEYPTR(hh, *r, e->alias, strlen(e->alias), e);
    }
}

SIValue Record_GetEntry(Record r, const char *alias) {
    Entry *e = NULL;
    HASH_FIND_STR(r, alias, e);
    assert(e);
    return e->v;
}

Node *Record_GetNode(const Record r, const char *alias) {
    SIValue entry = Record_GetEntry(r, alias);
    return (Node*)entry.ptrval;
}

Edge *Record_GetEdge(const Record r, const char *alias) {
    SIValue entry = Record_GetEntry(r, alias);
    return (Edge*)entry.ptrval;
}

void Record_Print(const Record r) {
    for(Entry *e = r; e != NULL; e = e->hh.next) {
        printf("alias %s:\n", e->alias);
    }
}

void Record_Free(Record r) {
    if(!r) return;

    Entry *e, *tmp;
    HASH_ITER(hh, r, e, tmp) {
        HASH_DEL(r, e);
        free(e);
    }
}
