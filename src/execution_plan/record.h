/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __RECORD_H_
#define __RECORD_H_

#include "../value.h"
#include "../graph/entities/node.h"
#include "../graph/entities/edge.h"

typedef enum  {
    REC_TYPE_SCALAR,
    REC_TYPE_NODE,
    REC_TYPE_EDGE,
    REC_TYPE_UNKNOWN,
} RecordEntryType;

typedef struct {
    union {
        SIValue s;
        Node n;
        Edge e;
    } value;
    RecordEntryType type;
} Entry;

typedef Entry* Record;

Record Record_New(int entries);

RecordEntryType Record_GetType(const Record r, int idx);

SIValue Record_GetScalar(const Record r, int idx);

Node *Record_GetNode(const Record r, int idx);

Edge *Record_GetEdge(const Record r, int idx);

GraphEntity *Record_GetGraphEntity(const Record r, int idx);

void Record_AddScalar(Record r, int idx, SIValue v);

void Record_AddNode(Record r, int idx, Node node);

void Record_AddEdge(Record r, int idx, Edge edge);

void Record_Print(const Record r);

void Record_Free(Record r);

#endif
