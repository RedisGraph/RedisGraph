#ifndef NODE_H_
#define NODE_H_

#include "rmutil/vector.h"
#include "filter.h"

// Describes a filter which will get applied to specified property.
typedef struct {
	char* property;	// property to filter.
	Filter* filter;
} PropertyFilter;

typedef struct {
	char* name;				// Node's name
	Vector* filters;		// List of filters
	Vector* outgoingEdges;	// List of outgoing edges
} Node;

// Creates a new node.
Node* NewNode(const char* name);

// Adds a filter to given node.
void NodeAddFilter(const Node* node, const char* property, Filter* filter);

// Validate checks the node for validity and returns false if something is wrong.
int ValidateNode(const Node* node);

// Frees alocated space by given node.
void FreeNode(Node* node);

#endif