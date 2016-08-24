#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "graph.h"

Node* NewNode(const char* name) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->filters = NewVector(Filter*, 0); // Vector initial capacity 0.
	node->incomingEdges = NewVector(Edge*, 0); // Vector initial capacity 0.
	node->outgoingEdges = NewVector(Edge*, 1); // Vector initial capacity 1.
	node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));

	strcpy(node->name, name);

	return node;
}

void NodeAddFilter(const Node* node, const Filter* filter) {
	Vector_Push(node->filters, filter);
}

Edge* ConnectNodes(const Node* a, const Node* b, const char* relationship) {
	Edge* edge = NewEdge(a, b, relationship);

	Vector_Push(a->outgoingEdges, edge);
	Vector_Push(b->incomingEdges, edge);

	return edge;
}

int ValidateNode(const Node* node) {
	// Check if node has no edges coming in or out of it.
	if(Vector_Size(node->incomingEdges) == 0 &&
		Vector_Size(node->outgoingEdges) == 0) {
		fprintf(stderr, "Disconnected, floating nodes are not allowed\n");
		return 0;
	}

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
	Vector_Free(node->incomingEdges);
	Vector_Free(node->outgoingEdges);
	free(node);
}

Edge* NewEdge(const Node* src, const Node* dest, const char* relationship) {
	Edge* edge = (Edge*)malloc(sizeof(Edge));
	
	edge->src = src;
	edge->dest = dest;	
	edge->relationship = (char*)malloc(strlen(relationship) + 1);
	strcpy(edge->relationship, relationship);

	return edge;
}

int ValidateEdge(const Edge* edge) {
	if(edge->src == 0) {
		fprintf(stderr, "edge missing source node\n");
		return 0;
	}

	if(edge->dest == 0) {
		fprintf(stderr, "edge missing destination node\n");
		return 0;
	}

	return 1;
}

void FreeEdge(Edge* edge) {
	free(edge->relationship);
	free(edge);
}