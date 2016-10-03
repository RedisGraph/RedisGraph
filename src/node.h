#ifndef NODE_H_
#define NODE_H_

typedef struct {
	char* name; // Node's name
} Node;

// Creates a new node.
Node* NewNode(const char* name);

// Frees alocated space by given node.
void FreeNode(Node* node);

#endif