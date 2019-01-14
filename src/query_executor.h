/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "redismodule.h"
#include "parser/ast.h"
#include "parser/newast.h"
#include "graph/query_graph.h"
#include "arithmetic/arithmetic_expression.h"

/* Modifies AST by expanding RETURN * or RETURN
 * a into a list of individual properties. */
void ExpandCollapsedNodes(AST *ast);

/* Create an AST from raw query. */
AST **ParseQuery(const char *query, size_t qLen, char **errMsg);

/* Make sure AST is valid. */
AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, AST **ast);

/* Performs a number of adjustments to given AST. */
void ModifyAST(GraphContext *gc, AST *ast, const cypher_parse_result_t *new_ast);

#endif
