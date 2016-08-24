#ifndef GRAPH_H
#define GRAPH_H

#include "filter.h"
#include "rmutil/vector.h"

#include "node.h"
#include "edge.h"

typedef struct {
	Vector* nodes;
} Graph;

// Creats a new graph
Graph* NewGraph();

// Adds a new node to the graph.
Node* GraphAddNode(const Graph* graph, const char* nodeName);

Node* GraphRemoveNode(Graph* graph, const char* nodeName);

// Retrives a node from the graph.
Node* GraphGetNode(const Graph* graph, const char* nodeName);

// Connects node a with an outgoing edge to node b, (A->B) returns connecting edge.
Edge* GraphConnectNodes(const Graph * graph, const char* a, const char* b, const char* relationship);

// Validate checks the graph for validity and returns false if something is wrong.
int ValidateGraph(const Graph* graph);

// Frees alocated space by given graph
void FreeGraph(Graph* graph);

#endif