#include "node.h"
#include "edge.h"	// Not sure about this include.

Node* NewNode(const char* name) {
	Node* node = (Node*)malloc(sizeof(Node));	
	node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));

	strcpy(node->name, name);

	return node;
}

void FreeNode(Node* node) {
	free(node->name);	
	free(node);
}