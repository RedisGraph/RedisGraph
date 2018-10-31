/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#define REDISMODULE_EXPERIMENTAL_API    // Required for block client.
#include "redismodule.h"

#include "parser/ast.h"
#include "graph/query_graph.h"
#include "arithmetic/arithmetic_expression.h"

/* Create an AST from raw query. */
AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg);

/* Make sure AST is valid. */
AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, AST_Query *ast);

/* Construct an expression tree foreach none aggregated term.
 * Returns a vector of none aggregated expression trees. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node,
                                                  AR_ExpNode ***expressions,
                                                  int *expressions_count);

/* Performs a number of adjustments to given AST. */
void ModifyAST(GraphContext *gc, AST_Query *ast);

#endif
