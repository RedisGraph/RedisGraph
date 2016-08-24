#ifndef GRAPH_H
#define GRAPH_H

#include "filter.h"
#include "rmutil/vector.h"

typedef struct {
	char* name;				// Node's name
	Vector* filters;		// List of filters
	Vector* incomingEdges;	// List of outgoing edges
	Vector* outgoingEdges;	// List of outgoing edges
} Node;

typedef struct {
	char* relationship;
	const Node* src;
	const Node* dest;

} Edge;

// Creates a new node.
Node* NewNode(const char* name);

// Adds a filter to given node.
void NodeAddFilter(const Node* node, const Filter* filter);

// Connects node a with an outgoing edge to node b, (A->B) returns connecting edge.
Edge* ConnectNodes(const Node* a, const Node* b, const char* relationship);

// Validate checks the node for validity and returns false if something is wrong with the filter
int ValidateNode(const Node* node);

// Frees alocated space by given filter.
void FreeNode(Node* node);

Edge* NewEdge(const Node* src, const Node* dest, const char* relationship);

int ValidateEdge(const Edge* edge);

void FreeEdge(Edge* edge);

#endif