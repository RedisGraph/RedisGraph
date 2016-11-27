#include "node.h"
#include "edge.h"

Node* NewNode(const char* alias, const char* id) {
	Node* node = (Node*)malloc(sizeof(Node));
	
	node->id = NULL;
	node->alias = NULL;
	node->outgoingEdges = NewVector(Edge*, 0);
	node->incomingEdges = 0;

	if(id != NULL) {
		node->id = (char*)malloc(sizeof(char) * (strlen(id) + 1));
		strcpy(node->id, id);
	}

	if(alias != NULL) {
		node->alias = (char*)malloc(sizeof(char) * (strlen(alias) + 1));
		strcpy(node->alias, alias);
	}

	return node;
}

void ConnectNode(Node* src, Node* dest, const char* connection) {
	Edge* e = NewEdge(src, dest, connection);
	Vector_Push(src->outgoingEdges, e);
	dest->incomingEdges++;
}

void FreeNode(Node* node) {
	free(node->id);
	free(node->alias);

	for(int i = 0; i < Vector_Size(node->outgoingEdges); i++) {
		Edge* e;
		Vector_Get(node->outgoingEdges, i, &e);
		FreeEdge(e);
	}

	Vector_Free(node->outgoingEdges);

	free(node);
}