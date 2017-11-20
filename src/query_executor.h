#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/graph.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "hexastore/triplet.h"
#include "arithmetic_expression.h"

/* Given AST's MATCH node constructs a graph
 * representing queried entities and the relationships
 * between them. */
void BuildGraph(Graph *graph, Vector *entities);

/* Constructs an arithmetic expression tree foreach none aggregated term. */
void Build_None_Aggregated_Arithmetic_Expressions(AST_ReturnNode *return_node, AR_ExpNode ***expressions, int *expressions_count, Graph* g);

int ReturnClause_ContainsAggregation(AST_QueryExpressionNode *ast);

/* Checks to see if return clause contains a collapsed node. */
int ReturnClause_ContainsCollapsedNodes(AST_QueryExpressionNode *ast);

void ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, const char *graphName);

/* Checks if query performs write (Create/Delete/Update) */
int Query_Modifies_KeySpace(const AST_QueryExpressionNode *ast);

AST_QueryExpressionNode* ParseQuery(const char *query, size_t qLen, char **errMsg);

#endif
