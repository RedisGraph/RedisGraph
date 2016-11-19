#ifndef GRAPH_H_
#define GRAPH_H_

#include "node.h"
#include "edge.h"
#include "../rmutil/vector.h"

typedef struct {
    Vector* nodes;
} Graph;

Graph* NewGraph();

// Search the graph for a node with given alias 
Node* Graph_GetNodeByAlias(const Graph* g, const char* alias);

// Adds a new node to the graph
Node* Graph_AddNode(Graph* g, const char* alias, const char* id);

// Finds a node with given input degree 
Node* Graph_GetNDegreeNode(Graph* g, int degree);

// Frees entire graph.
void FreeGraph(Graph* g);

#endif