#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ItemNode* NewItemNode(const char* alias, const char* id) {
	ItemNode* node = (ItemNode*)malloc(sizeof(ItemNode));

	node->id = (char*)malloc(strlen(id) + 1);
	node->alias = (char*)malloc(strlen(alias) + 1);

	strcpy(node->id, id);
	strcpy(node->alias, alias);

	return node;
}

void FreeItemNode(ItemNode* itemNode) {
	free(itemNode->id);
	free(itemNode->alias);
	free(itemNode);
}


LinkNode* NewLinkNode(const char* relationship) {
	LinkNode* node = (LinkNode*)malloc(sizeof(LinkNode));

	node->relationship = (char*)malloc(strlen(relationship) + 1);

	strcpy(node->relationship, relationship);

	return node;
}

void FreeLinkNode(LinkNode* linkNode) {
	free(linkNode->relationship);
	free(linkNode);
}


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
			free(filterNode->pn.alias);
			free(filterNode->pn.property);
		case N_COND:
			FreeFilterNode(filterNode->cn.left);
			FreeFilterNode(filterNode->cn.right);
	}	
}


MatchNode* NewMatchNode(RelationshipNode* relationshipNode) {
	MatchNode* matchNode = (MatchNode*)malloc(sizeof(MatchNode));
	matchNode->relationshipNode = relationshipNode;

	return matchNode;
}

void FreeMatchNode(MatchNode* matchNode) {
	FreeRelationshipNode(matchNode->relationshipNode);
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
	ReturnNode* returnNode = malloc(sizeof(ReturnNode));
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


VariableNode* CreateVariableNode(const char* alias, const char* property) {
	VariableNode* variableNode = (VariableNode*)malloc(sizeof(VariableNode));
	variableNode->alias = (char*)malloc(strlen(alias) + 1);
	variableNode->property = (char*)malloc(strlen(property) + 1);
	
	strcpy(variableNode->alias, alias);
	strcpy(variableNode->property, property);
	return variableNode;
}

void FreeVariableNode(VariableNode* variableNode) {
	free(variableNode->alias);
	free(variableNode->property);
	free(variableNode);
}


RelationshipNode* NewRelationshipNode(ItemNode* src, LinkNode* relation, ItemNode* dest) {
	RelationshipNode* relationshipNode = (RelationshipNode*)malloc(sizeof(RelationshipNode));
	relationshipNode->src = src;
	relationshipNode->relation = relation;
	relationshipNode->dest = dest;

	return relationshipNode;
}

void FreeRelationshipNode(RelationshipNode* relationshipNode) {
	FreeItemNode(relationshipNode->src);
	FreeItemNode(relationshipNode->dest);
	FreeLinkNode(relationshipNode->relation);
	free(relationshipNode);
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
	FreeReturnNode(queryExpressionNode->returnNode);
}
