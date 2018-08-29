/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _CLAUSE_WHERE_H
#define _CLAUSE_WHERE_H

#include "../../value.h"
#include "../../rmutil/vector.h"
#include "../../util/triemap/triemap.h"

typedef enum {
  N_PRED,
  N_COND,
} AST_FilterNodeType;

typedef enum {
	N_CONSTANT,
	N_VARYING,
} AST_CompareValueType;

struct filterNode;

typedef struct {
	union {
		SIValue constVal;
		struct {
			char *alias;
			char *property;
		} nodeVal;
	};
	AST_CompareValueType t; // Compared value type, constant/node
	char *alias;			// Node alias
	char *property; 		// Node property
	int op;					// Type of comparison
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
AST_FilterNode* New_AST_ConstantPredicateNode(const char *alias, const char *property, int op, SIValue value);
AST_FilterNode* New_AST_VaryingPredicateNode(const char *lAlias, const char *lProperty, int op, const char *rAlias, const char *rProperty);
AST_FilterNode* New_AST_ConditionNode(AST_FilterNode *left, int op, AST_FilterNode *right);
void WhereClause_ReferredEntities(const AST_WhereNode *where_node, TrieMap *referred_nodes);
void Free_AST_FilterNode(AST_FilterNode *filterNode);
void Free_AST_WhereNode(AST_WhereNode *whereNode);

#endif
