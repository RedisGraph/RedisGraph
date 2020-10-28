/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "qg_node.h"
#include "qg_edge.h"
#include "../graph.h"
#include "../../util/arr.h"
#include <assert.h>

static void _QGNode_RemoveEdge(QGEdge **edges, QGEdge *e) {
	uint edge_count = array_len(edges);
	for(uint i = 0; i < edge_count; i++) {
		QGEdge *ie = edges[i];
		if(e == edges[i]) {
			array_del_fast(edges, i);
			return;
		}
	}
}

QGNode *QGNode_New(const char *alias) {
	QGNode *n = rm_malloc(sizeof(QGNode));
	n->label = NULL;
	n->alias = alias;
	n->highly_connected = false;
	n->labelID = GRAPH_NO_LABEL;
	n->incoming_edges = array_new(QGEdge *, 0);
	n->outgoing_edges = array_new(QGEdge *, 0);
	return n;
}

bool QGNode_HighlyConnected(const QGNode *n) {
	return n->highly_connected;
}

int QGNode_Degree(const QGNode *n) {
	return QGNode_IncomeDegree(n) + QGNode_OutgoingDegree(n);
}

int QGNode_IncomeDegree(const QGNode *n) {
	return array_len(n->incoming_edges);
}

int QGNode_OutgoingDegree(const QGNode *n) {
	return array_len(n->outgoing_edges);
}

int QGNode_EdgeCount(const QGNode *n) {
	return QGNode_IncomeDegree(n) + QGNode_OutgoingDegree(n);
}

void QGNode_ConnectNode(QGNode *src, QGNode *dest, QGEdge *e) {
	src->outgoing_edges = array_append(src->outgoing_edges, e);
	dest->incoming_edges = array_append(dest->incoming_edges, e);

	// Set src node as highly connected if in-degree + out-degree > 2
	if(src->highly_connected == false && QGNode_Degree(src) > 2) {
		src->highly_connected = true;
	}

	// Set dest node as highly connected if in-degree + out-degree > 2
	if(dest->highly_connected == false && QGNode_Degree(dest) > 2) {
		dest->highly_connected = true;
	}
}

void QGNode_RemoveIncomingEdge(QGNode *n, QGEdge *e) {
	_QGNode_RemoveEdge(n->incoming_edges, e);
}

void QGNode_RemoveOutgoingEdge(QGNode *n, QGEdge *e) {
	_QGNode_RemoveEdge(n->outgoing_edges, e);
}

QGNode *QGNode_Clone(const QGNode *orig) {
	QGNode *n = rm_malloc(sizeof(QGNode));
	memcpy(n, orig, sizeof(QGNode));
	// Don't save edges when duplicating a node
	n->incoming_edges = array_new(QGEdge *, 0);
	n->outgoing_edges = array_new(QGEdge *, 0);

	return n;
}

int QGNode_ToString(const QGNode *n, char *buff, int buff_len) {
	assert(n && buff);

	int offset = 0;
	offset += snprintf(buff + offset, buff_len - offset, "(");
	if(n->alias) offset += snprintf(buff + offset, buff_len - offset, "%s", n->alias);
	if(n->label) offset += snprintf(buff + offset, buff_len - offset, ":%s", n->label);
	offset += snprintf(buff + offset, buff_len - offset, ")");
	return offset;
}

void QGNode_Free(QGNode *node) {
	if(!node) return;

	if(node->outgoing_edges) array_free(node->outgoing_edges);
	if(node->incoming_edges) array_free(node->incoming_edges);

	rm_free(node);
}

