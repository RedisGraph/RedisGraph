/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef QUERY_GRAPH_H_
#define QUERY_GRAPH_H_

#include "entities/node.h"
#include "entities/edge.h"
#include "graph.h"
#include "graphcontext.h"
#include "../util/vector.h"
#include "../resultset/resultset_statistics.h"

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
} QueryGraph;

/* Prepare a new query graph with initial allocations for
 * the provided node and edge counts. */
QueryGraph* QueryGraph_New(size_t node_cap, size_t edge_cap);

/* Given AST's MATCH node constructs a graph
 * representing queried entities and the relationships
 * between them. */
void BuildQueryGraph(const GraphContext *gc, QueryGraph *query_graph, Vector *entities);

/* Checks if graph contains given node */
bool QueryGraph_ContainsNode(const QueryGraph *graph, const Node *node);

/* Checks if graph contains given edge */
bool QueryGraph_ContainsEdge(const QueryGraph *graph, const Edge *edge);

/* Retrieves node from graph */
Node* QueryGraph_GetNodeById(const QueryGraph *g, long int id);

/* Retrieves node from graph */
Edge* QueryGraph_GetEdgeById(const QueryGraph *g, long int id);

/* Search the graph for a node with given alias */
Node* QueryGraph_GetNodeByAlias(const QueryGraph *g, const char *alias);

/* Search the graph for an edge with given alias */
Edge* QueryGraph_GetEdgeByAlias(const QueryGraph *g, const char *alias);

/* Search for either node/edge with given alias. */
GraphEntity* QueryGraph_GetEntityByAlias(const QueryGraph *g, const char *alias);

GraphEntity** QueryGraph_GetEntityRef(const QueryGraph *g, const char *alias);

Node** QueryGraph_GetNodeRef(const QueryGraph *g, const Node *n);

Edge** QueryGraph_GetEdgeRef(const QueryGraph *g, const Edge *e);

/* Adds a new node to the graph */
void QueryGraph_AddNode(QueryGraph* g, Node *n);

/* Remove given node from query graph. */
Node* QueryGraph_RemoveNode(QueryGraph *g, Node *n);

/* Adds a new edge to the graph */
void QueryGraph_ConnectNodes(QueryGraph *g, Node *src, Node *dest, Edge *e);

/* Remove given edge from query graph. */
Edge* QueryGraph_RemoveEdge(QueryGraph *g, Edge *e);

/* Performs deep copy of input query graph. */
QueryGraph* QueryGraph_Clone(const QueryGraph *g);

/* Breaks up query graph into its connected components.
 * Returns an array object */
QueryGraph** QueryGraph_ConnectedComponents(const QueryGraph *qg);

/* Frees entire graph */
void QueryGraph_Free(QueryGraph* g);

#endif
