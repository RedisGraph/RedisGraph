/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Forward declaration of edge */
struct QGEdge;

typedef struct {
	int labelID;               // Label ID
	const char *label;         // Label string
	const char *alias;         // User-provided alias associated with this node
	bool highly_connected;     // Node degree > 2
	struct QGEdge **outgoing_edges;   // Array of incoming edges (ME)<-(SRC)
	struct QGEdge **incoming_edges;   // Array of outgoing edges (ME)->(DEST)
} QGNode;

/* Creates a new node. */
QGNode *QGNode_New(const char *alias);

/* Returns true if node is highly connected, false otherwise */
bool QGNode_HighlyConnected(const QGNode *n);

/* Returns the number of both incoming and outgoing edges. */
int QGNode_Degree(const QGNode *n);

/* Returns number of edges pointing into node. */
int QGNode_IncomeDegree(const QGNode *n);

/* Returns number of edges pointing out of node. */
int QGNode_OutgoingDegree(const QGNode *n);

/* Returns to total number of edges (Incoming&Outgoing). */
int QGNode_EdgeCount(const QGNode *n);

/* Connects source node to destination node by edge. */
void QGNode_ConnectNode(QGNode *src, QGNode *dest, struct QGEdge *e);

/* Removes given Incoming edge from node. */
void QGNode_RemoveIncomingEdge(QGNode *n, struct QGEdge *e);

/* Removes given Outgoing edge from node. */
void QGNode_RemoveOutgoingEdge(QGNode *n, struct QGEdge *e);

/* Clones given node. */
QGNode *QGNode_Clone(const QGNode *n);

/* Gets a string representation of given node. */
int QGNode_ToString(const QGNode *n, char *buff, int buff_len);

/* Frees allocated space by given node. */
void QGNode_Free(QGNode *node);

