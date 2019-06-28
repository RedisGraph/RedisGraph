/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "qg_node.h"
#include "../graph.h"
#include "../../util/arr.h"
#include <assert.h>

QGNode* QGNode_New(const char *label, const char *alias) {
    QGNode *n = rm_malloc(sizeof(QGNode));
    n->n = Node_New(NULL, NULL); // label, alias
    n->alias = NULL;
    n->label = NULL;
    n->labelID = GRAPH_NO_LABEL;
    n->incoming_edges = array_new(QGEdge*, 0);
    n->outgoing_edges = array_new(QGEdge*, 0);
    // n->id = -1;
    return n;
}

void QGNode_ConnectNode(QGNode* src, QGNode* dest, QGEdge* e) {
	src->outgoing_edges = array_append(src->outgoing_edges, e);
	dest->incoming_edges = array_append(dest->incoming_edges, e);
}

void _QGNode_RemoveEdge(QGEdge **edges, QGEdge *e) {
	uint edge_count = array_len(edges);
	for(uint i = 0; i < edge_count; i++) {
		QGEdge *ie = edges[i];
		if(e == ie) {
			edges[i] = edges[edge_count-1];
			array_pop(edges);
			return;
		}
	}
}

void QGNode_RemoveIncomingEdge(QGNode *n, QGEdge *e) {
	_QGNode_RemoveEdge(n->incoming_edges, e);
}

void QGNode_RemoveOutgoingEdge(QGNode *n, QGEdge *e) {
	_QGNode_RemoveEdge(n->outgoing_edges, e);
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

void QGNode_SetLabelID(QGNode *n, int labelID) {
	assert(n);
	n->labelID = labelID;
}

// GrB_Matrix QGNode_GetMatrix(QGNode *n) {
	/* QGNode's label must be set, 
	 * otherwise it doesn't make sense to refer to a matrix. */
	// assert(n && n->label);

	// // Retrieve matrix from graph if edge matrix isn't set.
    // if(!n->mat) {
        // GraphContext *gc = GraphContext_GetFromTLS();
        // Graph *g = gc->g;

        /* Get label matrix:
		 * There's no sense in calling QGNode_GetMatrix
		 * if node isn't labeled. */
		// assert(n->labelID != GRAPH_NO_LABEL);
        // if(n->labelID == GRAPH_UNKNOWN_LABEL) {
			// // Label specified (n:Label), but doesn't exists.
			// n->mat = Graph_GetZeroMatrix(g);
		// } else {
			// n->mat = Graph_GetLabelMatrix(g, n->labelID);
        // }
    // }

    // return n->mat;
// }

QGNode* QGNode_Clone(const QGNode *orig) {
    QGNode *n = rm_malloc(sizeof(QGNode));
    n->n = orig->n;
    n->id = orig->id;
    n->label = orig->label;
    n->labelID = orig->labelID;
    n->alias = orig->alias;
    n->incoming_edges = array_new(QGEdge*, 0);
    n->outgoing_edges = array_new(QGEdge*, 0);

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

void QGNode_Free(QGNode* node) {
	if(!node) return;

	// if(node->outgoing_edges) array_free(node->outgoing_edges);
	// if(node->incoming_edges) array_free(node->incoming_edges);

	// rm_free(node);
	node = NULL;
}