#ifndef EDGE_H_
#define EDGE_H_

#include "node.h"

typedef struct {
	char* relationship;
	Node* src;
	Node* dest;
} Edge;


// Creates a new edge, connecting src to dest node.
Edge* NewEdge(Node* src, Node* dest, const char* relationship);

// Validate checks the edge for validity and returns false if something is wrong.
int ValidateEdge(const Edge* edge);

// Frees alocated space by given edge
void FreeEdge(Edge* edge);

#endif