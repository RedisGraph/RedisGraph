/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "qg_node.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

struct QGEdge {
	const char *alias;      /* User-provided alias attached to edge. */
	const char **reltypes;  /* Relationship type strings */
	int *reltypeIDs;        /* Relationship type IDs */ // TODO can be uint save for GRAPH_NO_RELATION
	QGNode *src;            /* Pointer to source node. */
	QGNode *dest;           /* Pointer to destination node. */
	uint minHops;           /* Minimum number of hops this edge represents. */
	uint maxHops;           /* Maximum number of hops this edge represents. */
	bool bidirectional;     /* Edge doesn't have a direction. */
};

typedef struct QGEdge QGEdge;

/* Creates a new edge without forming connections. */
QGEdge *QGEdge_New(const char *relationship, const char *alias);

/* Create a duplicate of an edge containing all of the original's data. */
QGEdge *QGEdge_Clone(const QGEdge *orig);

/* Determine whether this is a variable length edge. */
bool QGEdge_VariableLength(const QGEdge *e);

/* Reverse edge direction. */
void QGEdge_Reverse(QGEdge *e);

/* Gets a string representation of given edge. */
int QGEdge_ToString(const QGEdge *e, char *buff, int buff_len);

/* Free allocations associated with the given edge. */
void QGEdge_Free(QGEdge *e);

