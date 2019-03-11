/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <stdlib.h>
#include <assert.h>

#include "graph_entity.h"
#include "node.h"
#include "edge.h"

Node* Node_New(const char *label, const char *alias) {
	Node* n = calloc(1, sizeof(Node));
	n->outgoing_edges = NewVector(Edge*, 0);
	n->incoming_edges = NewVector(Edge*, 0);

	if(label != NULL) n->label = strdup(label);
	if(alias != NULL) n->alias = strdup(alias);

	return n;
}

int Node_Compare(const Node *a, const Node *b) {
	return ENTITY_GET_ID(a) == ENTITY_GET_ID(b);
}

void Node_ConnectNode(Node* src, Node* dest, struct Edge* e) {
	Vector_Push(src->outgoing_edges, e);
	Vector_Push(dest->incoming_edges, e);
}

int Node_IncomeDegree(const Node *n) {
	return Vector_Size(n->incoming_edges);
}

void Node_Print(const Node *node, FILE *out) {
    const char *fix(const char *p) { return p ? p : "''"; }
    if(!out) out = stdout;
    GraphEntity_Print((GraphEntity*) node, GETYPE_NODE, 0);
    if (node->label || node->alias)
        fprintf(out, "\tlabel: %s alias: %s\n", fix(node->label), fix(node->alias));
}

void Node_Free(Node* node) {
	if(!node) return;

	if(node->label != NULL) free(node->label);
	if(node->alias != NULL) free(node->alias);
	if(node->outgoing_edges) Vector_Free(node->outgoing_edges);
	if(node->incoming_edges) Vector_Free(node->incoming_edges);

	free(node);
	node = NULL;
}
