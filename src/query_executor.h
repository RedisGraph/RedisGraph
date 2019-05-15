/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "redismodule.h"
#include "ast/ast.h"
#include "graph/query_graph.h"
#include "arithmetic/arithmetic_expression.h"

/* Make sure AST is valid. */
AST_Validation AST_PerformValidations(RedisModuleCtx *ctx, const AST *ast);

AR_ExpNode** AST_BuildReturnExpressions(AST *ast, const cypher_astnode_t *ret_clause);
AR_ExpNode** AST_BuildWithExpressions(AST *ast, const cypher_astnode_t *with_clause);
AR_ExpNode** AST_BuildOrderExpressions(AST *ast, const cypher_astnode_t *order_clause);

#endif
