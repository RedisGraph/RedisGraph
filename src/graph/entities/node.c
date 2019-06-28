/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdlib.h>

#include "node.h"
#include "edge.h"
#include "assert.h"
#include "graph_entity.h"
#include "../../util/arr.h"
#include "../graphcontext.h"

Node* Node_New(const char *label, const char *alias) {
	Node* n = calloc(1, sizeof(Node));
	n->outgoing_edges = array_new(Edge*, 0);
	n->incoming_edges = array_new(Edge*, 0);
	n->labelID = GRAPH_UNKNOWN_LABEL;

    // TODO valid?
	n->label = label;
	n->alias = alias;
	// if(label != NULL) n->label = strdup(label);
	// if(alias != NULL) n->alias = strdup(alias);

	return n;
}

int Node_Compare(const Node *a, const Node *b) {
	return ENTITY_GET_ID(a) == ENTITY_GET_ID(b);
}

void Node_ConnectNode(Node* src, Node* dest, struct Edge* e) {
	src->outgoing_edges = array_append(src->outgoing_edges, e);
	dest->incoming_edges = array_append(dest->incoming_edges, e);
}

void _Node_RemoveEdge(Edge **edges, Edge *e) {
	size_t edge_count = array_len(edges);
	for(uint i = 0; i < edge_count; i++) {
		Edge *ie = edges[i];
		if(e == ie) {
			edges[i] = edges[edge_count-1];
			array_pop(edges);
			return;
		}
	}
}

void Node_RemoveIncomingEdge(Node *n, Edge *e) {
	_Node_RemoveEdge(n->incoming_edges, e);
}

void Node_RemoveOutgoingEdge(Node *n, Edge *e) {
	_Node_RemoveEdge(n->outgoing_edges, e);
}

int Node_IncomeDegree(const Node *n) {
	return array_len(n->incoming_edges);
}

int Node_OutgoingDegree(const Node *n) {
	return array_len(n->outgoing_edges);
}

int Node_EdgeCount(const Node *n) {
	return Node_IncomeDegree(n) + Node_OutgoingDegree(n);
}

void Node_SetLabelID(Node *n, int labelID) {
	assert(n);
	n->labelID = labelID;
}

GrB_Matrix Node_GetMatrix(Node *n) {
	/* Node's label must be set, 
	 * otherwise it doesn't make sense to refer to a matrix. */
	assert(n && n->label);

	// Retrieve matrix from graph if edge matrix isn't set.
    if(!n->mat) {
        GraphContext *gc = GraphContext_GetFromTLS();
        Graph *g = gc->g;

        /* Get label matrix:
		 * There's no sense in calling Node_GetMatrix
		 * if node isn't labeled. */
		assert(n->labelID != GRAPH_NO_LABEL);
        if(n->labelID == GRAPH_UNKNOWN_LABEL) {
			// Label specified (n:Label), but doesn't exists.
			n->mat = Graph_GetZeroMatrix(g);
		} else {
			n->mat = Graph_GetLabelMatrix(g, n->labelID);
        }
    }

    return n->mat;
}

Node* Node_Clone(const Node *n) {
	Node *clone = Node_New(n->label, n->alias);
	clone->mat = n->mat;
	// TODO: consider setting labelID in Node_New.
	clone->labelID = n->labelID;
	return clone;
}

int Node_ToString(const Node *n, char *buff, int buff_len) {
    assert(n && buff);

    int offset = 0;
    offset += snprintf(buff + offset, buff_len - offset, "(");
    if(n->alias) offset += snprintf(buff + offset, buff_len - offset, "%s", n->alias);
    if(n->label) offset += snprintf(buff + offset, buff_len - offset, ":%s", n->label);
    offset += snprintf(buff + offset, buff_len - offset, ")");
    return offset;
}

void Node_Free(Node* node) {
	if(!node) return;

	// if(node->label != NULL) free(node->label);
	// if(node->alias != NULL) free(node->alias);
	if(node->outgoing_edges) array_free(node->outgoing_edges);
	if(node->incoming_edges) array_free(node->incoming_edges);

	free(node);
	node = NULL;
}
