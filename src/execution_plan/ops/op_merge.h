/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_MERGE_H
#define __OP_MERGE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../parser/ast.h"
#include "../../resultset/resultset.h"

/* Merge execution plan operation,
 * this operation will create a pattern P if it doesn't not exists
 * in the graph, the pattern must be either fully matched, in which case
 * we'll simply return, otherwise (no match/ partial match) the ENTIRE pattern
 * is created. */

typedef struct {
    OpBase op;              // Base op.
    Graph *g;               // Graph been queried.
    QueryGraph *qg;         // Query graph.
    AST_Query *ast;         // Query abstract syntax tree.
    RedisModuleCtx *ctx;    // Redis context.
    Node **nodes;           // Original query-graph nodes.
    Edge **edges;           // Original query-graph edges.
    ResultSet *result_set;  // Required for statistics updates.
    const char *graph_name; // Required to fetch label stores from keyspace.
    bool request_refresh;   // Request refresh on first call to consume.
    bool matched;           // Was the entire pattern been matched.
} OpMerge;

OpBase* NewMergeOp(RedisModuleCtx *ctx, AST_Query *ast, Graph *g, QueryGraph *qg, const char *graph_name, ResultSet *result_set);
OpResult OpMergeConsume(OpBase *opBase, QueryGraph* graph);
OpResult OpMergeReset(OpBase *ctx);
void OpMergeFree(OpBase *ctx);

#endif
