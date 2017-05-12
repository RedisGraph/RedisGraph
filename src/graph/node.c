#include "node.h"
#include "edge.h"
#include <stdlib.h>

Node* NewNode(const char* alias, const char* id) {
	Node* node = (Node*)calloc(1, sizeof(Node));
	
	node->id = NULL;
	node->alias = NULL;
	node->outgoingEdges = NewVector(Edge*, 0);
	node->incomingEdges = NewVector(Edge*, 0);
	node->internalId = rand();

	if(id != NULL) {
		node->id = strdup(id);
	}

	if(alias != NULL) {
		node->alias = strdup(alias);
	}

	return node;
}

Node* Node_Clone(const Node *node) {
	Node *clone = NewNode(node->alias, node->id);
	clone->internalId = node->internalId;
	return clone;
}

int Node_Compare(const Node *a, const Node *b) {
	return a->internalId == b->internalId;
}

void ConnectNode(Node* src, Node* dest, struct Edge* e) {
	Vector_Push(src->outgoingEdges, e);
	Vector_Push(dest->incomingEdges, e);
}

int Node_IncomeDegree(const Node *n) {
	return Vector_Size(n->incomingEdges);
}

void FreeNode(Node* node) {
	if(node->id != NULL) {
		free(node->id);
	}
	if(node->alias != NULL) {
		free(node->alias);
	}

	for(int i = 0; i < Vector_Size(node->outgoingEdges); i++) {
		Edge* e;
		Vector_Get(node->outgoingEdges, i, &e);
		FreeEdge(e);
	}
	Vector_Free(node->outgoingEdges);

	/* There no need to discard incoming edges.
	* these will be freed on another outgoingEdges free. */ 
	Vector_Free(node->incomingEdges);
	free(node);
}