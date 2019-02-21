/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _CLAUSE_WHERE_H
#define _CLAUSE_WHERE_H

#include "../../value.h"
#include "../../util/vector.h"
#include "../../util/triemap/triemap.h"
#include "../ast_arithmetic_expression.h"

typedef enum {
  N_PRED,
  N_COND,
} AST_FilterNodeType;

struct filterNode;

typedef struct {
  AST_ArithmeticExpressionNode *lhs;  // Left-hand expression
  AST_ArithmeticExpressionNode *rhs;  // Right-hand expression
  int op;                             // Type of comparison
} AST_PredicateNode;

typedef struct conditionNode {
  struct filterNode *left;
  struct filterNode *right;
  int op;
} AST_ConditionNode;

typedef struct filterNode {
  union {
    AST_PredicateNode pn;
    AST_ConditionNode cn;
  };
  AST_FilterNodeType t;
} AST_FilterNode;

typedef struct {
	AST_FilterNode *filters;
} AST_WhereNode;

AST_WhereNode* New_AST_WhereNode(AST_FilterNode *filters);
AST_FilterNode* New_AST_PredicateNode(AST_ArithmeticExpressionNode *lhs, int op, AST_ArithmeticExpressionNode *rhs);
AST_FilterNode* New_AST_ConditionNode(AST_FilterNode *left, int op, AST_FilterNode *right);
void WhereClause_ReferredEntities(const AST_WhereNode *where_node, TrieMap *referred_entities);
void WhereClause_ReferredFunctions(const AST_FilterNode *return_node, TrieMap *referred_funcs);
void Free_AST_FilterNode(AST_FilterNode *filterNode);
void Free_AST_WhereNode(AST_WhereNode *whereNode);

#endif
