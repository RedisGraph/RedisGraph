/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "edge.h"
#include "qg_node.h"
#include <string.h>

struct QGEdge {
    Edge *e;
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

// Retrieve edge source node ID.
// NodeID QGEdge_GetSrcNodeID(const QGEdge *edge);

// // Retrieve edge destination node ID.
// NodeID QGEdge_GetDestNodeID(const QGEdge *edge);

// // Retrieve edge relation ID.
// int QGEdge_GetRelationID(const QGEdge *edge);

// Retrieve edge source node.
QGNode* QGEdge_GetSrcNode(QGEdge *e);

// Retrieve edge destination node.
QGNode* QGEdge_GetDestNode(QGEdge *e);

// Retrieves edge matrix.
GrB_Matrix QGEdge_GetMatrix(QGEdge *e);

// Determins if this is a variable length edge.
bool QGEdge_VariableLength(const QGEdge *e);

// Reverse edge direction.
void QGEdge_Reverse(QGEdge *e);

// Sets edge source node.
// void QGEdge_SetSrcNode(QGEdge *e, QGNode *src);

// // Sets edge destination node.
// void QGEdge_SetDestNode(QGEdge *e, QGNode *dest);

// // Sets edge relation type.
// void QGEdge_SetRelationID(QGEdge *e, int relationID);

// Gets a string representation of given edge.
int QGEdge_ToString(const QGEdge *e, char *buff, int buff_len);

// Frees allocated space by given edge
void QGEdge_Free(QGEdge *edge);