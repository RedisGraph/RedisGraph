/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./where.h"
#include <assert.h>

AST_WhereNode* New_AST_WhereNode(AST_FilterNode *filters) {
	AST_WhereNode *whereNode = (AST_WhereNode*)malloc(sizeof(AST_WhereNode));
	whereNode->filters = filters;
	return whereNode;
}

AST_FilterNode* New_AST_PredicateNode(AST_ArithmeticExpressionNode *lhs, int op, AST_ArithmeticExpressionNode *rhs) {
	AST_FilterNode *n = malloc(sizeof(AST_FilterNode));
  n->t = N_PRED;
  n->pn.lhs = lhs;
  n->pn.op = op;
  n->pn.rhs = rhs;

  return n;
}

AST_FilterNode *New_AST_ConditionNode(AST_FilterNode *left, int op, AST_FilterNode *right) {
  AST_FilterNode *n = malloc(sizeof(AST_FilterNode));
  n->t = N_COND;
  n->cn.left = left;
  n->cn.right = right;
  n->cn.op = op;

  return n;
}

void FreePredicateNode(AST_PredicateNode* predicateNode) {

  // TODO free, but maybe leave arithmetic expressions alone
  /*
	if(predicateNode->alias) {
		free(predicateNode->alias);
	}

	if(predicateNode->property) {
		free(predicateNode->property);
	}

	if(predicateNode->t == N_VARYING) {
		if(predicateNode->nodeVal.alias) {
			free(predicateNode->nodeVal.alias);
		}

		if(predicateNode->nodeVal.property) {
			free(predicateNode->nodeVal.property);
		}
	}
  */

	// TODO: Should I free constVal?
}

void _WhereClause_ReferredNodes(AST_FilterNode *root, TrieMap *referred_nodes) {
	switch(root->t) {
		case N_COND:
			_WhereClause_ReferredNodes(root->cn.left, referred_nodes);
			_WhereClause_ReferredNodes(root->cn.right, referred_nodes);
			break;
		case N_PRED:
      AR_EXP_GetAliases(root->pn.lhs, referred_nodes);
      AR_EXP_GetAliases(root->pn.rhs, referred_nodes);
			break;
		default:
			assert(0);
			break;
	}
}

void WhereClause_ReferredNodes(const AST_WhereNode *where_node, TrieMap *referred_nodes) {
	if(!where_node) return;
	_WhereClause_ReferredNodes(where_node->filters, referred_nodes);
}

void Free_AST_FilterNode(AST_FilterNode* filterNode) {
	if(!filterNode)
		return;

	switch(filterNode->t) {
		case N_PRED:
			FreePredicateNode(&filterNode->pn);
			break;
		case N_COND:
			Free_AST_FilterNode(filterNode->cn.left);
			Free_AST_FilterNode(filterNode->cn.right);
			break;
	}
}

void Free_AST_WhereNode(AST_WhereNode *whereNode) {
	if(!whereNode) return;

	Free_AST_FilterNode(whereNode->filters);
	free(whereNode);
}
