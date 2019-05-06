/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "entities/node.h"
#include "entities/edge.h"
#include "graphcontext.h"
#include "../parser/ast.h"
#include "../parser/ast_shared.h"

typedef struct {
    // TODO both arrays are only used for choosing the proper free routine
    // right now - should refactor to remove.
    // Could just have separate triemaps, since I don't think there's any
    // real benefit to an integrated one.
    Node **nodes;
    Edge **edges;
    TrieMap *ast_references;
} QueryGraph;

/* Prepare a new query graph with initial allocations for
 * the provided node and edge counts. */
QueryGraph* QueryGraph_New(size_t node_cap, size_t edge_cap);

/* Add all nodes and relationships from a single path
 * (from part of a MATCH or CREATE pattern, or a MERGE clause)
 * to the QueryGraph. */
void QueryGraph_AddPath(const GraphContext *gc, const AST *ast, QueryGraph *qg, const cypher_astnode_t *path);

/* Adds all paths described in an AST pattern node (from a
 * MATCH or CREATE clause) to a meta-graph that describes all
 * nodes and relationships in a query. */
QueryGraph* BuildQueryGraph(const GraphContext *gc, const AST *ast);

/* Adds a new edge to the graph */
void QueryGraph_ConnectNodes(QueryGraph *qg, Node *src, Node *dest, Edge *e, char *edge_alias);

/* Retrieve a graph entity from an AST pointer */
void* QueryGraph_GetEntityByASTRef(const QueryGraph *qg, const cypher_astnode_t *ref);

/* Frees entire graph */
void QueryGraph_Free(QueryGraph* qg);
