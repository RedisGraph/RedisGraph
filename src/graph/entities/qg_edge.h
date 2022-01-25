/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
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

// creates a new edge without forming connections
QGEdge *QGEdge_New
(
	const char *relationship,
	const char *alias
);

// returns edge alias
const char *QGEdge_Alias
(
	const QGEdge *e
);

// returns edge source node
QGNode *QGEdge_Src
(
	const QGEdge *e
);

// returns edge destination node
QGNode *QGEdge_Dest
(
	const QGEdge *e
);

// create a duplicate of an edge containing all of the original's data
QGEdge *QGEdge_Clone
(
	const QGEdge *orig
);

// determine whether this is a variable length edge
bool QGEdge_VariableLength
(
	const QGEdge *e
);

// number of relationships associated with edge
int QGEdge_RelationCount
(
	const QGEdge *e
);

// return relationship for relation at position 'idx'
const char *QGEdge_Relation
(
	const QGEdge *e,
	int idx
);

// return relationship id for relation at position 'idx'
int QGEdge_RelationID
(
	const QGEdge *e,
	int idx
);

// reverse edge direction
void QGEdge_Reverse
(
	QGEdge *e
);

// gets a string representation of given edge
void QGEdge_ToString
(
	const QGEdge *e,
	sds *buff
);

// free allocations associated with the given edge
void QGEdge_Free
(
	QGEdge *e
);

