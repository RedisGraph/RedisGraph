#ifndef NODE_H_
#define NODE_H_

#include "../rmutil/vector.h"
#include <uuid/uuid.h>

typedef struct {
	char* alias;
	char* id;
	uuid_t internalId;
	Vector* outgoingEdges;
	int incomingEdges;
} Node;

// Creates a new node.
Node* NewNode(const char* alias, const char* id);

// Connects source node to destination node using connection
void ConnectNode(Node* src, Node* dest, const char* connection);

// Frees alocated space by given node.
void FreeNode(Node* node);

#endif