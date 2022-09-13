/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include "../../util/sds/sds.h"
#include "qg_node.h"
#include "qg_edge.h"

typedef struct {
	const char *alias;  // user-provided alias associated with this path
	QGNode **nodes;     // array of nodes
	QGEdge **edges;     // array of edges
} QGPath;

// creates a new path
QGPath *QGPath_New
(
	const char *alias
);

// returns the alias of the path
const char *QGPath_Alias
(
	const QGPath *p
);

// add node to path
void QGPath_AddNode
(
	QGPath *p,
	QGNode *n
);

// add edge to path
void QGPath_AddEdge
(
	QGPath *p,
	QGEdge *e
);

// clones given path
QGPath *QGPath_Clone
(
	const QGPath *p
);

// gets a string representation of given path
void QGPath_ToString
(
	const QGPath *p,
	sds *buff
);

// frees allocated space by given path
void QGPath_Free
(
	QGPath *p
);

