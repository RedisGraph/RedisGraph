/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _FILTER_TREE_H
#define _FILTER_TREE_H

#include "../parser/newast.h"
#include "../redismodule.h"
#include "../arithmetic/arithmetic_expression.h"
#include "../execution_plan/record.h"

#define FILTER_FAIL 0
#define FILTER_PASS 1

/* Nodes within the filter tree are one of two types
 * Either a predicate node or a condition node. */
typedef enum {
  FT_N_PRED,
  FT_N_COND,
} FT_FilterNodeType;

struct FT_FilterNode;

typedef struct {
	AR_ExpNode *lhs;
	AR_ExpNode *rhs;
	AST_Operator op;	/* Can validly be an operation (<, <=, =, =>, >, <>, maybe NOT). */
} FT_PredicateNode;

typedef struct {
	struct FT_FilterNode *left;
	struct FT_FilterNode *right;
	AST_Operator op;	/* Can validly be OR, AND (and later, XOR and maybe NOT) */
} FT_ConditionNode;

/* All nodes within the filter tree are of type FT_FilterNode. */
struct FT_FilterNode {
  union {
    FT_PredicateNode pred;
    FT_ConditionNode cond;
  };
  FT_FilterNodeType t;	/* Determines actual type of this node. */
};

typedef struct FT_FilterNode FT_FilterNode;

FT_FilterNode* FilterNode_FromAST(const NEWAST *ast, const cypher_astnode_t *expr);

/* Given AST's WHERE subtree constructs a filter tree
 * This is done to speed up the filtering process. */
FT_FilterNode* BuildFiltersTree(const NEWAST *ast);

int IsNodePredicate(const FT_FilterNode *node);

FT_FilterNode* CreateCondFilterNode(AST_Operator op);

FT_FilterNode *AppendLeftChild(FT_FilterNode *root, FT_FilterNode *child);
FT_FilterNode *AppendRightChild(FT_FilterNode *root, FT_FilterNode *child);

/* Runs val through the filter tree. */
int FilterTree_applyFilters(const FT_FilterNode* root, const Record r);

/* Extract every alias mentioned in the tree
 * without duplications. */
Vector *FilterTree_CollectAliases(const FT_FilterNode *root);

/* Prints tree. */
void FilterTree_Print(const FT_FilterNode *root);

/* Break filter tree into sub filter trees as follows:
 * sub trees under an OR operator are returned,
 * sub trees under an AND operator are broken down to the smallest
 * components possible following the two rules above. */
Vector* FilterTree_SubTrees(const FT_FilterNode *root);

void FilterTree_Free(FT_FilterNode *root);

#endif // _FILTER_TREE_H 