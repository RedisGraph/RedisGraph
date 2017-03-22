#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "graph/graph.h"
#include "value_cmp.h"
#include "parser/ast.h"
#include "redismodule.h"
#include "hexastore/triplet.h"

// Nodes within the filter tree are one of two types
// either a predicate node
// or a condition node
typedef enum {
  QE_N_PRED,
  QE_N_COND,
} QE_FilterNodeType;

typedef enum {
	QE_N_CONSTANT,
	QE_N_VARYING,
} QE_CompareValueType;

struct QE_FilterNode;

typedef struct {
	struct {			// Left side of predicate
		char* alias;		// Element in question alias
		char* property;		// Element's property to check
	} Lop;
	int op;					// Operation (<, <=, =, =>, >, !)
	union {					// Right side of predicate
		SIValue constVal;	// Value to compare against
		struct {
			char* alias;
			char* property;
		} Rop;
	};
	QE_CompareValueType t; 	// Comapred value type, constant/node
	CmpFunc cf;				// Compare function, determins relation between val and element property
} QE_PredicateNode;


typedef struct {
	struct QE_FilterNode *left;
	struct QE_FilterNode *right;
	int op;					// OR, AND
} QE_ConditionNode;

// All nodes within the filter tree are of type QE_FilterNode
typedef struct {
  union {
    QE_PredicateNode pred;
    QE_ConditionNode cond;
  };
  QE_FilterNodeType t;	// Determins actual type of this node
} QE_FilterNode;


// Given AST's MATCH node constructs a graph
// representing queried entities and the relationships
// between them
Graph* BuildGraph(const MatchNode* matchNode);

// Given AST's WHERE subtree constructs a filter tree
// this is done to speed up the filtering process
QE_FilterNode* BuildFiltersTree(const FilterNode* root);

// Runs val through the filter tree
int applyFilters(RedisModuleCtx *ctx, Graph* g, QE_FilterNode* root);

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

Triplet* FastSkipTriplets(RedisModuleCtx *ctx, const QE_FilterNode *filterTree, TripletIterator *iterator, Graph *g, Node *src, Node *dest);

QueryExpressionNode* ParseQuery(const char *query, size_t qLen, char **errMsg);

#endif
