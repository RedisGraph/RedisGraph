#ifndef __QUERY_EXECUTOR_H
#define __QUERY_EXECUTOR_H

#include "triplet.h"
#include "graph/graph.h"
#include "value_cmp.h"
#include "parser/ast.h"
#include "redismodule.h"

// Nodes within the filter tree are one of two types
// either a predicate node
// or a condition node
typedef enum {
  QE_N_PRED,
  QE_N_COND,
} QE_FilterNodeType;

struct QE_FilterNode;

typedef struct {
	char* alias;	// Element in question alias
	char* property;	// Element's property to check 
	int op;			// Operation (<, <=, =, =>, >, !)
	SIValue val;	// Value to compare against
	CmpFunc cf;	// Compare function, determins relation between val and element property
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
int applyFilters(RedisModuleCtx *ctx, Triplet* result, char** aliases, QE_FilterNode* root);
int applyFilter(RedisModuleCtx *ctx, const char* elementID, QE_PredicateNode* node);

#endif