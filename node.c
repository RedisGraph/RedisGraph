#include "node.h"
#include "edge.h"	// Not sure about this include.

Node* NewNode(const char* name) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->filters = NewVector(Filter*, 0); // Vector initial capacity 0.
	node->outgoingEdges = NewVector(Edge*, 0); // Vector initial capacity 0.
	node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));

	strcpy(node->name, name);

	return node;
}

void NodeAddFilter(const Node* node, const Filter* filter) {
	Vector_Push(node->filters, filter);
}

int ValidateNode(const Node* node) {
	// Check if node's filters are valid.
	for (int i = 0; i < Vector_Size(node->filters); i++) {
        Filter *f;
        Vector_Get(node->filters, i, &f);
        if(ValidateFilter(f) == 0) {
        	return 0;
        }
    }

	return 1;
}

void FreeNode(Node* node) {
	free(node->name);

	// Free filters
	for (int i = 0; i < Vector_Size(node->filters); i++) {
        Filter* f;
        Vector_Get(node->filters, i, &f);
        FreeFilter(f);        
    }
    
	Vector_Free(node->filters);
	Vector_Free(node->outgoingEdges);
	free(node);
}