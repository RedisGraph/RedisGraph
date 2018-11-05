/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_MERGE_H
#define __OP_MERGE_H

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../parser/ast.h"
#include "../../resultset/resultset.h"

/* Merge execution plan operation,
 * this operation will create a pattern P if it doesn't not exists
 * in the graph, the pattern must be either fully matched, in which case
 * we'll simply return, otherwise (no match/ partial match) the ENTIRE pattern
 * is created. */

typedef struct {
    OpBase op;              // Base op.
    GraphContext *gc;       // Graph data.
    QueryGraph *qg;         // Query graph.
    AST_Query *ast;         // Query abstract syntax tree.
    ResultSet *result_set;  // Required for statistics updates.
    bool matched;           // Has the entire pattern been matched.
} OpMerge;

OpBase* NewMergeOp(GraphContext *gc, AST_Query *ast, QueryGraph *qg, ResultSet *result_set);
OpResult OpMergeConsume(OpBase *opBase, Record *r);
OpResult OpMergeReset(OpBase *ctx);
void OpMergeFree(OpBase *ctx);

#endif
