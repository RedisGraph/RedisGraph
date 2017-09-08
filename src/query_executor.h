#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/graph.h"
#include "value_cmp.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "hexastore/triplet.h"

/* Given AST's MATCH node constructs a graph
 * representing queried entities and the relationships
 * between them. */
Graph* BuildGraph(const AST_MatchNode *matchNode);

/* Retrieves requested properties from the graph. */
Vector* ReturnClause_RetrievePropValues(const AST_ReturnNode *returnNode, const Graph *g);

/* Retrieves all properties which define the group key from given graph. */
Vector* ReturnClause_RetrieveGroupKeys(const AST_ReturnNode *returnNode, const Graph *g);

/* Retrieves all aggregated properties from graph. */
Vector* ReturnClause_RetrieveGroupAggVals(const AST_ReturnNode *returnNode, const Graph *g);

int ReturnClause_ContainsAggregation(const AST_ReturnNode *returnNode);

/* Retrieves all aggregation functions used within given return clause.
 * Returns a vector of AggCtx* */
Vector* ReturnClause_GetAggFuncs(RedisModuleCtx *ctx, const AST_ReturnNode *returnNode);

/* Checks to see if return clause contains a collapsed node. */
int ReturnClause_ContainsCollapsedNodes(const AST_ReturnNode *returnNode);

void ReturnClause_ExpandCollapsedNodes(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, const char *graphName);

AST_QueryExpressionNode* ParseQuery(const char *query, size_t qLen, char **errMsg);

#endif
