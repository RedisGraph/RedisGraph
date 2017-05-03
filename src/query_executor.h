#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/graph.h"
#include "value_cmp.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "hexastore/triplet.h"

// Given AST's MATCH node constructs a graph
// representing queried entities and the relationships
// between them
Graph* BuildGraph(const MatchNode* matchNode);

// Retrieves property value from hashed element.
void GetElementProperyValue(RedisModuleCtx *ctx, const char *elementID, const char *property, RedisModuleString **propValue);

// Retrieves requested properties from the graph.
Vector* ReturnClause_RetrievePropValues(RedisModuleCtx *ctx, const ReturnNode *returnNode, const Graph *g);

// Retrieves all properties which define the group key from given graph.
Vector* ReturnClause_RetrieveGroupKeys(RedisModuleCtx *ctx, const ReturnNode *returnNode, const Graph *g);

// Retrieves all aggregated properties from graph.
Vector* ReturnClause_RetrieveGroupAggVals(RedisModuleCtx *ctx, const ReturnNode *returnNode, const Graph *g);

int ReturnClause_ContainsAggregation(const ReturnNode *returnNode);

// Retrieves all aggregation functions used within given return clause.
// Returns a vector of AggCtx*
Vector* ReturnClause_GetAggFuncs(RedisModuleCtx *ctx, const ReturnNode *returnNode);

// Checks to see if return clause contains a collapsed node.
int ReturnClause_ContainsCollapsedNodes(const ReturnNode *returnNode);

void ReturnClause_ExpendCollapsedNodes(RedisModuleCtx *ctx, ReturnNode *returnNode, RedisModuleString *graphName, Graph *graph);

QueryExpressionNode* ParseQuery(const char *query, size_t qLen, char **errMsg);

#endif
