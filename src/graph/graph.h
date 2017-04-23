#ifndef GRAPH_H_
#define GRAPH_H_

#include "node.h"
#include "edge.h"
#include "../rmutil/vector.h"
#include "../hexastore/hexastore.h"

typedef struct {
    Vector *nodes;
} Graph;

Graph* NewGraph();

Graph* Graph_Clone(const Graph *graph);

/* Checks if graphs are equal
 * Returns 0 if a and b are the same
 * 1 otherwise */ 
int Graph_Compare(const Graph *a, const Graph *b);

/* Checks if graph contains given node
 * Returns 1 if so, 0 otherwise */ 
int Graph_ContainsNode(const Graph *graph, const Node *node);

/* Search the graph for a node with given alias */
Node* Graph_GetNodeByAlias(const Graph *g, const char *alias);

/* Adds a new node to the graph */
int Graph_AddNode(Graph *g, Node *n);

/* Finds a node with given input degree */
Vector* Graph_GetNDegreeNodes(Graph *g, int degree);

/* Finds the shortest path between source and destination nodes */
Graph* Graph_ShortestPath(Graph *g, Node *src, Node *dest);

/* Frees entire graph */
void Graph_Free(Graph* g);

#endif