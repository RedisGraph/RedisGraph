/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "redismodule.h"
#include "parser/ast.h"
#include "graph/query_graph.h"
#include "arithmetic/arithmetic_expression.h"

/* Modifies AST by expanding RETURN * or RETURN
 * a into a list of individual properties. */
void ExpandCollapsedNodes(AST *ast);

/* Make sure AST is valid. */
AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, const AST *ast);

void AST_BuildReturnExpressions(AST *ast);
void AST_BuildWithExpressions(AST *ast);

#endif
