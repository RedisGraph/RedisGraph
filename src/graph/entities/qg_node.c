/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "qg_node.h"
#include "qg_edge.h"
#include "RG.h"
#include "../../util/arr.h"

static void _QGNode_RemoveEdge(QGEdge **edges, QGEdge *e) {
	uint edge_count = array_len(edges);
	for(uint i = 0; i < edge_count; i++) {
		QGEdge *ie = edges[i];
		if(e == ie) {
			array_del_fast(edges, i);
			return;
		}
	}
}

QGNode *QGNode_New(const char *alias) {
	QGNode *n = rm_malloc(sizeof(QGNode));
	n->alias             =  alias;
	n->labels            =  array_new(const char *, 0);
	n->labelsID          =  array_new(int, 0);
	n->incoming_edges    =  array_new(QGEdge *, 0);
	n->outgoing_edges    =  array_new(QGEdge *, 0);
	n->highly_connected  =  false;
	return n;
}

bool QGNode_Labeled(const QGNode *n) {
	ASSERT(n != NULL);

	return array_len(n->labels) > 0;
}

uint QGNode_LabelCount(const QGNode *n) {
	ASSERT(n != NULL);

	return array_len(n->labels);
}

int QGNode_GetLabelID(const QGNode *n, uint idx) {
	ASSERT(n != NULL);
	ASSERT(idx < QGNode_LabelCount(n));

	return n->labelsID[idx];
}

const char *QGNode_GetLabel(const QGNode *n, uint idx) {
	ASSERT(n != NULL);
	ASSERT(idx < QGNode_LabelCount(n));

	return n->labels[idx];
}

bool QGNode_HasLabel(const QGNode *n, const char *l) {
	ASSERT(n != NULL);

	uint label_count = QGNode_LabelCount(n);
	for(uint i = 0; i < label_count; i++) {
		if(strcmp(n->labels[i], l) == 0) return true;
	}

	return false;
}

void QGNode_AddLabel(QGNode *n, const char *l, int l_id) {
	ASSERT(n != NULL);
	ASSERT(l != NULL);

	// node already labeled as l, no work to be done.
	if(QGNode_HasLabel(n, l)) return;

	n->labels = array_append(n->labels, l);
	n->labelsID = array_append(n->labelsID, l_id);
}

bool QGNode_HighlyConnected(const QGNode *n) {
	ASSERT(n != NULL);

	return n->highly_connected;
}

int QGNode_Degree(const QGNode *n) {
	ASSERT(n != NULL);

	return QGNode_IncomeDegree(n) + QGNode_OutgoingDegree(n);
}

int QGNode_IncomeDegree(const QGNode *n) {
	ASSERT(n != NULL);

	return array_len(n->incoming_edges);
}

int QGNode_OutgoingDegree(const QGNode *n) {
	ASSERT(n != NULL);

	return array_len(n->outgoing_edges);
}

int QGNode_EdgeCount(const QGNode *n) {
	ASSERT(n != NULL);

	return QGNode_IncomeDegree(n) + QGNode_OutgoingDegree(n);
}

void QGNode_ConnectNode(QGNode *src, QGNode *dest, QGEdge *e) {
	ASSERT(e != NULL);
	ASSERT(src != NULL);
	ASSERT(dest != NULL);

	src->outgoing_edges = array_append(src->outgoing_edges, e);
	dest->incoming_edges = array_append(dest->incoming_edges, e);

	// set src node as highly connected if in-degree + out-degree > 2
	if(src->highly_connected == false && QGNode_Degree(src) > 2) {
		src->highly_connected = true;
	}

	// set dest node as highly connected if in-degree + out-degree > 2
	if(dest->highly_connected == false && QGNode_Degree(dest) > 2) {
		dest->highly_connected = true;
	}
}

void QGNode_RemoveIncomingEdge(QGNode *n, QGEdge *e) {
	ASSERT(n != NULL);
	ASSERT(e != NULL);

	_QGNode_RemoveEdge(n->incoming_edges, e);
}

void QGNode_RemoveOutgoingEdge(QGNode *n, QGEdge *e) {
	ASSERT(n != NULL);
	ASSERT(e != NULL);

	_QGNode_RemoveEdge(n->outgoing_edges, e);
}

QGNode *QGNode_Clone(const QGNode *orig) {
	ASSERT(orig != NULL);

	QGNode *clone = rm_malloc(sizeof(QGNode));
	memcpy(clone, orig, sizeof(QGNode));

	array_clone(clone->labels, orig->labels);
	array_clone(clone->labelsID, orig->labelsID);

	// don't clone edges when duplicating a node
	clone->incoming_edges = array_new(QGEdge *, 0);
	clone->outgoing_edges = array_new(QGEdge *, 0);

	return clone;
}

int QGNode_ToString(const QGNode *n, char *buff, int buff_len) {
	ASSERT(n && buff);

	int offset = 0;
	offset += snprintf(buff + offset, buff_len - offset, "(");
	if(n->alias) {
		offset += snprintf(buff + offset, buff_len - offset, "%s", n->alias);
	}
	for(uint i = 0; i < QGNode_LabelCount(n); i++) {
		offset += snprintf(buff + offset, buff_len - offset, ":%s", n->labels[i]);
	}
	offset += snprintf(buff + offset, buff_len - offset, ")");
	return offset;
}

void QGNode_Free(QGNode *node) {
	if(!node) return;

	array_free(node->labels);
	array_free(node->labelsID);
	array_free(node->outgoing_edges);
	array_free(node->incoming_edges);

	rm_free(node);
}

