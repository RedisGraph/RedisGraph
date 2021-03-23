/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "qg_node.h"
#include "../../path_patterns/ebnf.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* Type of QGEdge instances.
 * QG_RELATION_PATTERN corresponds to original QGEdge like ()-[...]-()
 * QG_PATH_PATTERN corresponds to path patterns like ()-/.../-() */
typedef enum {
    QG_RELATION_PATTERN,
    QG_PATH_PATTERN,
} QGPatternType;


/* Now QGEdge represents either relation or path pattern. */
struct QGEdge {
    // Common part
    QGPatternType type;     /* Type of pattern: either relation (original behavior)
 							 * or path pattern (new behavior). */

	QGNode *src;            /* Pointer to source node. */
	QGNode *dest;           /* Pointer to destination node. */
	bool bidirectional;     /* Edge doesn't have a direction. */
	const char *alias;      /* User-provided alias attached to edge. */

	/* Relation pattern specific part */
	const char **reltypes;  /* Relationship type strings */
	int *reltypeIDs;        /* Relationship type IDs */ // TODO can be uint save for GRAPH_NO_RELATION
	uint minHops;           /* Minimum number of hops this edge represents. */
	uint maxHops;           /* Maximum number of hops this edge represents. */

    /* Path pattern specific part */
    EBNFBase *pattern;      /* Expression of path pattern, that represents
                             * its structure (like abstract syntax tree of some expression) */
};

typedef struct QGEdge QGEdge;

/* Creates a new edge (relation pattern), connecting src to dest node. */
QGEdge *QGEdge_NewRelationPattern(QGNode *src, QGNode *dest, const char *relationship, const char *alias);

/* Creates a new edge (path pattern pattern), connecting src to dest node. */
QGEdge *QGEdge_NewPathPattern(QGNode *src, QGNode *dest, EBNFBase *pattern);

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
