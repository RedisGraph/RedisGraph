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
  int lhs_type = AR_EXP_GetOpType(lhs);
  int rhs_type = AR_EXP_GetOpType(rhs);
  if (lhs_type == AST_AR_EXP_VARIADIC) {
    if (rhs_type == AST_AR_EXP_VARIADIC) {
      // me.age > friend.age
      return New_AST_VaryingPredicateNode(lhs->operand.variadic.alias, lhs->operand.variadic.property, op, rhs->operand.variadic.alias, rhs->operand.variadic.property);
    } else if (rhs_type == AST_AR_EXP_CONSTANT) {
      // me.age > 30
      return New_AST_ConstantPredicateNode(lhs->operand.variadic.alias, lhs->operand.variadic.property, op, rhs->operand.constant);
    }
  } else if (lhs_type == 0) {
    // Function on left hand of filter
    /* TODO Ultimately, it makes sense to support functions against variadics as well as functions
     * against functions, but I'm currently only adding (arithmetic) functions against constants. */
    if (rhs_type == AST_AR_EXP_CONSTANT) {
      // ID(a) = 5
      return New_AST_FunctionPredicateNode(lhs->op, op, rhs->operand.constant);
    }
  }

  return NULL;
}

AST_FilterNode* New_AST_FunctionPredicateNode(AST_ArithmeticExpressionOP func, int op, SIValue value) {
	AST_FilterNode *n = malloc(sizeof(AST_FilterNode));
  n->t = N_PRED;

  n->pn.t = N_FUNC;
  // TODO logic needs to be updated for multi-argument functions (WHERE LEFT(a.name, 3) = 'The') 
  assert(Vector_Size(func.args) == 1);
  n->pn.func.function = strdup(func.function);
  AST_ArithmeticExpressionNode *exp = NULL;
  Vector_Get(func.args, 0, &exp);
  assert(exp->type == AST_AR_EXP_OPERAND);
  assert(exp->operand.type == AST_AR_EXP_VARIADIC);

  n->pn.alias = strdup(exp->operand.variadic.alias);
  if (exp->operand.variadic.property) {
    n->pn.property = strdup(exp->operand.variadic.property);
  } else {
    n->pn.property = NULL;
  }
  n->pn.op = op;
	n->pn.func.constVal = value;

  return n;
}

AST_FilterNode* New_AST_ConstantPredicateNode(const char* alias, const char* property, int op, SIValue value) {
	AST_FilterNode *n = malloc(sizeof(AST_FilterNode));
  	n->t = N_PRED;

	n->pn.t = N_CONSTANT;
  	n->pn.alias = strdup(alias);
	n->pn.property = strdup(property);

	n->pn.op = op;
	n->pn.constVal = value;

	return n;
}

AST_FilterNode* New_AST_VaryingPredicateNode(const char* lAlias, const char* lProperty, int op, const char* rAlias, const char* rProperty) {
	AST_FilterNode *n = malloc(sizeof(AST_FilterNode));
	n->t = N_PRED;

	n->pn.t = N_VARYING;
	n->pn.alias = (char*)malloc(strlen(lAlias) + 1);
	n->pn.property = (char*)malloc(strlen(lProperty) + 1);
	n->pn.nodeVal.alias = (char*)malloc(strlen(rAlias) + 1);
	n->pn.nodeVal.property = (char*)malloc(strlen(rProperty) + 1);

	strcpy(n->pn.alias, lAlias);
	strcpy(n->pn.property, lProperty);
	strcpy(n->pn.nodeVal.alias, rAlias);
	strcpy(n->pn.nodeVal.property, rProperty);

	n->pn.op = op;

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

	// TODO: Should I free constVal?
}

void _WhereClause_ReferredNodes(AST_FilterNode *root, TrieMap *referred_nodes) {
	switch(root->t) {
		case N_COND:
			_WhereClause_ReferredNodes(root->cn.left, referred_nodes);
			_WhereClause_ReferredNodes(root->cn.right, referred_nodes);
			break;
		case N_PRED:
			TrieMap_Add(referred_nodes,
						root->pn.alias,
						strlen(root->pn.alias),
						NULL,
						TrieMap_DONT_CARE_REPLACE);
			if(root->pn.t == N_VARYING) {
				TrieMap_Add(referred_nodes,
							root->pn.nodeVal.alias,
							strlen(root->pn.nodeVal.alias),
							NULL,
							TrieMap_DONT_CARE_REPLACE);
			}
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
