#ifndef NODE_H_
#define NODE_H_

#include "../rmutil/vector.h"

typedef struct {
	char* alias;
	char* id;
	int internalId;
	Vector* outgoingEdges;
	int incomingEdges;
} Node;

// Creates a new node.
Node* NewNode(const char* alias, const char* id);

// Creates a clone of given node.
Node* Node_Clone(const Node *node);

// Checks if nodes are "equal"
int Node_Compare(const Node *a, const Node *b);

// Connects source node to destination node using connection
void ConnectNode(Node* src, Node* dest, const char* connection);

// Frees alocated space by given node.
void FreeNode(Node* node);

#endif