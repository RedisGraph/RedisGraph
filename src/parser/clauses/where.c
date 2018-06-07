#include "./where.h"
#include <assert.h>

AST_WhereNode* New_AST_WhereNode(AST_FilterNode *filters) {
	AST_WhereNode *whereNode = (AST_WhereNode*)malloc(sizeof(AST_WhereNode));
	whereNode->filters = filters;
	return whereNode;
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
