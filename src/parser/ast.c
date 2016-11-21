#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FilterNode* NewPredicateNode(const char* alias, const char* property, int op, SIValue value) {
	FilterNode *n = malloc(sizeof(FilterNode));
  	n->t = N_PRED;

  	n->pn.alias = (char*)malloc(strlen(alias) + 1);
	n->pn.property = (char*)malloc(strlen(property) + 1);

	strcpy(n->pn.alias, alias);
	strcpy(n->pn.property, property);

	n->pn.op = op;
	n->pn.val = value;

	return n;
}


FilterNode *NewConditionNode(FilterNode *left, int op, FilterNode *right) {
  FilterNode *n = malloc(sizeof(FilterNode));
  n->t = N_COND;
  n->cn.left = left;
  n->cn.right = right;
  n->cn.op = op;

  return n;
}

void FreeFilterNode(FilterNode* filterNode) {
	if(!filterNode)
		return;

	switch(filterNode->t) {
		case N_PRED:
			if(filterNode->pn.alias) {
				free(filterNode->pn.alias);
			}

			if(filterNode->pn.property) {
				free(filterNode->pn.property);
			}
			break;
		case N_COND:
			FreeFilterNode(filterNode->cn.left);
			FreeFilterNode(filterNode->cn.right);
			break;
	}
}


ChainElement* NewChainLink(char* relationship, LinkDirection dir) {
	ChainElement* ce = (ChainElement*)malloc(sizeof(ChainElement));
	ce->t = N_LINK;	
	
	ce->l.direction = dir;
	ce->l.relationship = (char*)malloc(strlen(relationship) + 1);
	strcpy(ce->l.relationship, relationship);

	return ce;
}


ChainElement* NewChainEntity(char* alias, char* id) {
	ChainElement* ce = (ChainElement*)malloc(sizeof(ChainElement));
	ce->t = N_ENTITY;
	
	ce->e.id = (char*)malloc(strlen(id) + 1);
	ce->e.alias = (char*)malloc(strlen(alias) + 1);

	strcpy(ce->e.id, id);
	strcpy(ce->e.alias, alias);

	return ce;
}

void FreeChainElement(ChainElement* chainElement) {
	switch(chainElement->t) {
		case N_ENTITY:
			free(chainElement->e.id);
			free(chainElement->e.alias);
			break;

		case N_LINK:
			free(chainElement->l.relationship);
			break;
	}

	free(chainElement);
}

MatchNode* NewMatchNode(Vector* elements) {
	MatchNode* matchNode = (MatchNode*)malloc(sizeof(MatchNode));
	matchNode->chainElements = elements;

	return matchNode;
}

void FreeMatchNode(MatchNode* matchNode) {
	for(int i = 0; i < Vector_Size(matchNode->chainElements); i++) {
		ChainElement* ce;
		Vector_Get(matchNode->chainElements, i, &ce);
		FreeChainElement(ce);
	}

	Vector_Free(matchNode->chainElements);
	free(matchNode);
}


WhereNode* NewWhereNode(FilterNode* filters) {
	WhereNode* whereNode = (WhereNode*)malloc(sizeof(WhereNode));
	whereNode->filters = filters;
	return whereNode;
}

void FreeWhereNode(WhereNode* whereNode) {
	if(!whereNode) {
		return;
	}

	FreeFilterNode(whereNode->filters);
	free(whereNode);
}


ReturnNode* NewReturnNode(Vector* variables) {
	ReturnNode* returnNode = (ReturnNode*)malloc(sizeof(ReturnNode));
	returnNode->variables = variables;

	return returnNode;
}

void FreeReturnNode(ReturnNode* returnNode) {
	for (int i = 0; i < Vector_Size(returnNode->variables); i++) {
		VariableNode *node;
		Vector_Get(returnNode->variables, i, &node);
		FreeVariableNode(node);
	}
	
	Vector_Free(returnNode->variables);
	free(returnNode);
}


VariableNode* NewVariableNode(const char* alias, const char* property) {
	VariableNode* variableNode = (VariableNode*)malloc(sizeof(VariableNode));
	variableNode->alias = NULL;
	variableNode->property = NULL;

	if(alias) {
		variableNode->alias = (char*)malloc(strlen(alias) + 1);
		strcpy(variableNode->alias, alias);
	}

	if(property) {
		variableNode->property = (char*)malloc(strlen(property) + 1);
		strcpy(variableNode->property, property);
	}
	
	return variableNode;
}

void FreeVariableNode(VariableNode* variableNode) {

	if(variableNode->alias){
		free(variableNode->alias);
	}
	
	if(variableNode->property) {

		free(variableNode->property);
	}

	free(variableNode);
}


QueryExpressionNode* NewQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode) {
	QueryExpressionNode* queryExpressionNode = (QueryExpressionNode*)malloc(sizeof(QueryExpressionNode));
	
	queryExpressionNode->matchNode = matchNode;
	queryExpressionNode->whereNode = whereNode;
	queryExpressionNode->returnNode = returnNode;

	return queryExpressionNode;
}

void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode) {
	FreeMatchNode(queryExpressionNode->matchNode);
	FreeWhereNode(queryExpressionNode->whereNode);
	// FreeReturnNode(queryExpressionNode->returnNode);
}
