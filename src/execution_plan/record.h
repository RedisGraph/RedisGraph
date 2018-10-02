/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __RECORD_H_
#define __RECORD_H_

#include "../value.h"
#include "../util/uthash.h"
#include "../graph/node.h"
#include "../graph/edge.h"

typedef struct {
    const char *alias;
    SIValue v;
    UT_hash_handle hh;  // makes this structure hashable.
} Entry;

typedef Entry* Record;

Record Record_Empty();

void Record_AddEntry(Record *r, const char* k, SIValue v);

SIValue Record_GetEntry(const Record r, const char *alias);

Node *Record_GetNode(const Record r, const char *alias);

Edge *Record_GetEdge(const Record r, const char *alias);

void Record_Print(const Record r);

void Record_Free(Record r);

#endif
