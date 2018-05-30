#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/query_graph.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "arithmetic/arithmetic_expression.h"

/* Constructs an arithmetic expression tree foreach none aggregated term. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count, QueryGraph *g);

void ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, AST_Query *ast, const char *graphName);

/* Checks if query performs write (Create/Delete/Update) */
int Query_Modifies_KeySpace(const AST_Query *ast);

AST_Query* ParseQuery(const char *query, size_t qLen, char **errMsg);

#endif
