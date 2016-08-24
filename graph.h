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

typedef struct {
	Vector* nodes;
	Vector* edges;
} Graph;


// Creates a new node.
Node* NewNode(const char* name);

// Adds a filter to given node.
void NodeAddFilter(const Node* node, const Filter* filter);

// Validate checks the node for validity and returns false if something is wrong.
int ValidateNode(const Node* node);

// Frees alocated space by given node.
void FreeNode(Node* node);

// Creates a new edge, connecting src to dest node.
Edge* NewEdge(const Node* src, const Node* dest, const char* relationship);

// Validate checks the edge for validity and returns false if something is wrong.
int ValidateEdge(const Edge* edge);

// Frees alocated space by given edge
void FreeEdge(Edge* edge);

// Creats a new graph
Graph* NewGraph();

// Adds a new node to the graph.
Node* GraphAddNode(const Graph* graph, const char* nodeName);

// Retrives a node from the graph.
Node* GraphGetNode(const Graph* graph, const char* nodeName);

// Connects node a with an outgoing edge to node b, (A->B) returns connecting edge.
Edge* GraphConnectNodes(const Graph * graph, const char* a, const char* b, const char* relationship);

// Validate checks the graph for validity and returns false if something is wrong.
int ValidateGraph(const Graph* graph);

// Frees alocated space by given graph
void FreeGraph(Graph* graph);

#endif