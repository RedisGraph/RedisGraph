/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "entities/node.h"
#include "entities/edge.h"
#include "entities/qg_node.h"
#include "entities/qg_edge.h"
#include "graphcontext.h"
#include "../ast/ast.h"
#include "../ast/ast_shared.h"

typedef struct {
	QGNode **nodes;             // Nodes contained in QueryGraph
	QGEdge **edges;             // Edges contained in QueryGraph
} QueryGraph;

typedef enum {
	ENTITY_UNKNOWN,
	ENTITY_NODE,
	ENTITY_EDGE,
} EntityType;

/* Prepare a new query graph with initial allocations for
 * the provided node and edge counts. */
QueryGraph *QueryGraph_New(uint node_cap, uint edge_cap);

/* Adds a new node to the graph */
void QueryGraph_AddNode(QueryGraph *g, QGNode *n);

/* Adds a new edge to the graph */
void QueryGraph_ConnectNodes(QueryGraph *qg, QGNode *src, QGNode *dest, QGEdge *e);

/* Add all nodes and relationships from a single path
 * (from part of a MATCH or CREATE pattern, or a MERGE clause)
 * to the QueryGraph. */
void QueryGraph_AddPath(const GraphContext *gc, QueryGraph *qg, const cypher_astnode_t *path);

/* Adds all paths described in an AST pattern node (from a
 * MATCH or MERGE clause) to a meta-graph that describes all
 * nodes and relationships in a query. */
QueryGraph *BuildQueryGraph(const GraphContext *gc, const AST *ast);

/* Retrieve node by alias */
QGNode *QueryGraph_GetNodeByAlias(const QueryGraph *qg, const char *alias);

/* Retrieve edge by alias */
QGEdge *QueryGraph_GetEdgeByAlias(const QueryGraph *qg, const char *alias);

/* Determine whether a given alias refers to a node or relation. */
EntityType QueryGraph_GetEntityTypeByAlias(const QueryGraph *qg, const char *alias);

/* Performs deep copy of input query graph. */
QueryGraph *QueryGraph_Clone(const QueryGraph *g);

/* Remove given node from query graph. */
QGNode *QueryGraph_RemoveNode(QueryGraph *g, QGNode *n);

/* Remove given edge from query graph. */
QGEdge *QueryGraph_RemoveEdge(QueryGraph *g, QGEdge *e);

/* Breaks up query graph into its connected components.
 * Returns an array object */
QueryGraph **QueryGraph_ConnectedComponents(const QueryGraph *qg);

/* Retrieve the number of nodes in a QueryGraph. */
uint QueryGraph_NodeCount(const QueryGraph *qg);

/* Retrieve the number of edges in a QueryGraph. */
uint QueryGraph_EdgeCount(const QueryGraph *qg);

/* Frees entire graph */
void QueryGraph_Free(QueryGraph *qg);
