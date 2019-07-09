/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "qg_node.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

struct QGEdge {
    const char *alias;
    const char **reltypes;
    int *reltypeIDs; // TODO can be uint save for GRAPH_NO_RELATION
    QGNode *src;
    QGNode *dest;
    uint minHops;   /* Minimum number of hops this edge represents. */
    uint maxHops;   /* Maximum number of hops this edge represents. */
    uint id;
};

typedef struct QGEdge QGEdge;

/* Creates a new edge, connecting src to dest node. */
QGEdge* QGEdge_New(QGNode *src, QGNode *dest, const char *relationship, const char *alias);

QGEdge* QGEdge_Clone(const QGEdge *orig);

// Determins if this is a variable length edge.
bool QGEdge_VariableLength(const QGEdge *e);

// Reverse edge direction.
void QGEdge_Reverse(QGEdge *e);

// Gets a string representation of given edge.
int QGEdge_ToString(const QGEdge *e, char *buff, int buff_len);

// Frees allocated space by given edge
void QGEdge_Free(QGEdge *edge);
