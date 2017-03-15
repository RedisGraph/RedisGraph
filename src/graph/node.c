#include "node.h"
#include "edge.h"

Node* NewNode(const char* alias, const char* id) {
	Node* node = (Node*)malloc(sizeof(Node));
	
	node->id = NULL;
	node->alias = NULL;
	node->outgoingEdges = NewVector(Edge*, 0);
	node->incomingEdges = 0;
	uuid_generate(node->internalId);

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
	memcpy(&clone->internalId, &node->internalId, sizeof(node->internalId));
	return clone;
}

int Node_Compare(const Node *a, const Node *b) {
	return a->internalId == b->internalId;
}

void ConnectNode(Node* src, Node* dest, const char* connection) {
	Edge* e = NewEdge(src, dest, connection);
	Vector_Push(src->outgoingEdges, e);
	dest->incomingEdges++;
}

void FreeNode(Node* node) {
	if(node->id) {
		free(node->id);
	}
	if(node->alias) {
		free(node->alias);
	}

	for(int i = 0; i < Vector_Size(node->outgoingEdges); i++) {
		Edge* e;
		Vector_Get(node->outgoingEdges, i, &e);
		FreeEdge(e);
	}

	Vector_Free(node->outgoingEdges);

	free(node);
}