#ifndef _FILTER_TREE_H
#define _FILTER_TREE_H

#include "../value_cmp.h"
#include "../parser/ast.h"
#include "../graph/graph.h"
#include "../redismodule.h"

#define FILTER_FAIL 0
#define FILTER_PASS 1
// Nodes within the filter tree are one of two types
// either a predicate node
// or a condition node
typedef enum {
  FT_N_PRED,
  FT_N_COND,
} FT_FilterNodeType;

typedef enum {
	FT_N_CONSTANT,
	FT_N_VARYING,
} FT_CompareValueType;

struct FT_FilterNode;

typedef struct {
	struct {			    // Left side of predicate
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
	FT_CompareValueType t; 	// Comapred value type, constant/node
	CmpFunc cf;				// Compare function, determins relation between val and element property
} FT_PredicateNode;

typedef struct {
	struct FT_FilterNode *left;
	struct FT_FilterNode *right;
	int op;					// OR, AND
} FT_ConditionNode;

// All nodes within the filter tree are of type FT_FilterNode
typedef struct {
  union {
    FT_PredicateNode pred;
    FT_ConditionNode cond;
  };
  FT_FilterNodeType t;	// Determins actual type of this node
} FT_FilterNode;

// Given AST's WHERE subtree constructs a filter tree
// this is done to speed up the filtering process
FT_FilterNode* BuildFiltersTree(const FilterNode* root);

// Runs val through the filter tree
int applyFilters(RedisModuleCtx *ctx, Graph* g, FT_FilterNode* root);

// Checks to see if aliased node is within the filter tree.
int FilterTree_ContainsNode(const FT_FilterNode *root, const char *alias);

void FilterTree_Free(FT_FilterNode *root);

#endif // _FILTER_TREE_H 