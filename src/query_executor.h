/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/query_graph.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "arithmetic/arithmetic_expression.h"

/* Query context, used for concurent query processing. */
typedef struct {
    RedisModuleBlockedClient *bc;   // Blocked client.
    AST_Query* ast;                 // Parsed AST.
    RedisModuleString *graphName;   // Graph ID.
    double tic[2];                  // timings.
} QueryContext;

QueryContext* QueryContext_New(RedisModuleCtx *ctx, RedisModuleBlockedClient *bc, AST_Query* ast, RedisModuleString *graphName);

void QueryContext_Free(QueryContext* ctx);

/* Constructs an arithmetic expression tree foreach none aggregated term. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count, QueryGraph *g);

AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg);

/* Performs a number of adjustments to given AST. */
AST_Validation ModifyAST(RedisModuleCtx *ctx, AST_Query *ast, const char *graph_name, char **reason);

#endif
