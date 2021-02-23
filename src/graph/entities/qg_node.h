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
	int *labelsID;             // Labels ID array
	const char **labels;       // Labels string array
	const char *alias;         // User-provided alias associated with this node
	bool highly_connected;     // Node degree > 2
	struct QGEdge **outgoing_edges;   // Array of incoming edges (ME)<-(SRC)
	struct QGEdge **incoming_edges;   // Array of outgoing edges (ME)->(DEST)
} QGNode;

/* Creates a new node. */
QGNode *QGNode_New(const char *alias);

/* Returns true if the node is labeled. */
bool QGNode_Labeled(const QGNode *n);

/* Returns number of labels attached to node */
uint QGNode_LabelCount(const QGNode *n);

/* Returns the 'idx' label ID of 'n' */
int QGNode_LabelID(const QGNode *n, uint idx);

/* Returns the 'idx' label of 'n' */
const char *QGNode_Label(const QGNode *n, uint idx);

/* Returns true if 'n' has label 'l' */
bool QGNode_HasLabel(const QGNode *n, const char *l);

/* Label 'n' as 'l' */
void QGNode_AddLabel(QGNode *n, const char *l, int l_id);

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

