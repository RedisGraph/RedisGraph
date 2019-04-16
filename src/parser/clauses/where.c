/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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
	Free_AST_ArithmeticExpressionNode(predicateNode->lhs);
	Free_AST_ArithmeticExpressionNode(predicateNode->rhs);
	free(predicateNode);
}

void _WhereClause_ReferredEntities(AST_FilterNode *root, TrieMap *referred_entities) {
	switch(root->t) {
		case N_COND:
			_WhereClause_ReferredEntities(root->cn.left, referred_entities);
			_WhereClause_ReferredEntities(root->cn.right, referred_entities);
			break;
		case N_PRED:
			AST_AR_EXP_GetAliases(root->pn.lhs, referred_entities);
			AST_AR_EXP_GetAliases(root->pn.rhs, referred_entities);
			break;
		default:
			assert(0);
			break;
	}
}

void WhereClause_ReferredEntities(const AST_WhereNode *where_node, TrieMap *referred_entities) {
	if(!where_node) return;
	_WhereClause_ReferredEntities(where_node->filters, referred_entities);
}

void WhereClause_ReferredFunctions(const AST_FilterNode *root, TrieMap *referred_funcs) {
	if (!root) return;
	if (root->t == N_PRED) {
		// Check expressions on each side of predicate filters
		AST_ArithmeticExpressionNode *exp = root->pn.lhs;
		AST_AR_EXP_GetFunctions(exp, referred_funcs);
		exp = root->pn.rhs;
		AST_AR_EXP_GetFunctions(exp, referred_funcs);
	} else {
		// Visit both children of conditional nodes
		WhereClause_ReferredFunctions(root->cn.left, referred_funcs);
		WhereClause_ReferredFunctions(root->cn.right, referred_funcs);
	}
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
