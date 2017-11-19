#ifndef GRAPH_H_
#define GRAPH_H_

#include "node.h"
#include "edge.h"
#include "../rmutil/vector.h"
#include "../hexastore/hexastore.h"

#define DEFAULT_GRAPH_CAP 32 /* Number of edges/nodes within the graph. */

typedef struct {
    Node **nodes;
    Edge **edges;
    char **node_aliases;
    char **edge_aliases;
    size_t node_count;
    size_t edge_count;
    size_t node_cap;
    size_t edge_cap;
} Graph;

Graph* NewGraph();
Graph* NewGraph_WithCapacity(size_t node_cap, size_t edge_cap);

/* Checks if graph contains given node
 * Returns 1 if so, 0 otherwise */ 
int Graph_ContainsNode(const Graph *graph, const Node *node);

int Graph_ContainsEdge(const Graph *graph, const Edge *edge);

/* Retrieves node from graph */
Node* Graph_GetNodeById(const Graph *g, long int id);

/* Retrieves node from graph */
Edge* Graph_GetEdgeById(const Graph *g, long int id);

/* Search the graph for a node with given alias */
Node* Graph_GetNodeByAlias(const Graph *g, const char *alias);

/* Search the graph for an edge with given alias */
Edge* Graph_GetEdgeByAlias(const Graph *g, const char *alias);

/* Search for either node/edge with given alias. */
GraphEntity* Graph_GetEntityByAlias(const Graph *g, const char *alias);

/* Adds a new node to the graph */
int Graph_AddNode(Graph* g, Node *n, char *alias);

/* Adds a new edge to the graph */
void Graph_ConnectNodes(Graph *g, Node *src, Node *dest, Edge *e, char *edge_alias);

/* Finds a node with given input degree */
Vector* Graph_GetNDegreeNodes(Graph *g, int degree);

/* Looks up node's alias within the graph */
char* Graph_GetNodeAlias(const Graph *g, const Node *n);

/* Looks up edge's alias within the graph */
char* Graph_GetEdgeAlias(const Graph *g, const Edge *e);

GraphEntity** Graph_GetEntityRef(const Graph *g, const GraphEntity *entity);
Node** Graph_GetNodeRef(const Graph *g, const Node *n);
Edge** Graph_GetEdgeRef(const Graph *g, const Edge *e);

/* Frees entire graph */
void Graph_Free(Graph* g);

#endif