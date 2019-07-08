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

// struct QGEdge;


typedef struct {
    // TODO both arrays are only used for choosing the proper free routine
    // right now - should refactor to remove.
    // Could just have separate triemaps, since I don't think there's any
    // real benefit to an integrated one.
    QGNode **nodes;
    QGEdge **edges;
    TrieMap *ast_references;
} QueryGraph;

void QueryGraph_AddNode(QueryGraph *qg, QGNode *n);
void QueryGraph_AddEdge(QueryGraph *qg, QGEdge *n);

/* Prepare a new query graph with initial allocations for
 * the provided node and edge counts. */
QueryGraph* QueryGraph_New(uint node_cap, uint edge_cap);

/* Add all nodes and relationships from a single path
 * (from part of a MATCH or CREATE pattern, or a MERGE clause)
 * to the QueryGraph. */
void QueryGraph_AddPath(const GraphContext *gc, const AST *ast, QueryGraph *qg, const cypher_astnode_t *path);

/* Adds all paths described in an AST pattern node (from a
 * MATCH or MERGE clause) to a meta-graph that describes all
 * nodes and relationships in a query. */
QueryGraph* BuildQueryGraph(const GraphContext *gc, const AST *ast);

/* Add all paths described in CREATE clauses to the QueryGraph. */
void QueryGraph_AddCreateClauses(const GraphContext *gc, const AST *ast, QueryGraph *qg);

/* Adds a new node to the graph */
// void QueryGraph_AddNode(QueryGraph* g, Node *n);

/* Remove given node from query graph. */
QGNode* QueryGraph_RemoveNode(QueryGraph *g, QGNode *n);

/* Adds a new edge to the graph */
// void QueryGraph_ConnectNodes(QueryGraph *g, Node *src, Node *dest, Edge *e);
void QueryGraph_ConnectNodes(QueryGraph *qg, QGNode *src, QGNode *dest, QGEdge *e);

/* Remove given edge from query graph. */
QGEdge* QueryGraph_RemoveEdge(QueryGraph *g, QGEdge *e);

/* Performs deep copy of input query graph. */
QueryGraph* QueryGraph_Clone(const QueryGraph *g);

/* Removes all nodes and edges from query graph. */
void QueryGraph_Clear(QueryGraph *q);

/* Breaks up query graph into its connected components.
 * Returns an array object */
QueryGraph** QueryGraph_ConnectedComponents(const QueryGraph *qg);

/* Retrieve a graph entity from an AST pointer */
void* QueryGraph_GetEntityByASTRef(const QueryGraph *qg, const cypher_astnode_t *ref);

/* Retrieve a graph entity from an AST ID. */
void* QueryGraph_GetEntityByASTID(const QueryGraph *qg, uint id);

QGNode* QueryGraph_GetNodeByID(const QueryGraph *qg, uint id);

QGEdge* QueryGraph_GetEdgeByID(const QueryGraph *qg, uint id);

/* Determine whether a given alias refers to a node or relation. */
SchemaType QueryGraph_GetEntityTypeByAlias(const QueryGraph *qg, const char *alias);

/* Frees entire graph */
void QueryGraph_Free(QueryGraph* qg);
