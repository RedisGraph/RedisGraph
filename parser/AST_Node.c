#include "AST_Node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ItemNode* CreateItemNode(const char* alias, const char* id) {
	printf("CreateItemNode id: %s alias: %s\n", id, alias);
	ItemNode* node = (ItemNode*)malloc(sizeof(ItemNode));

	node->id = (char*)malloc(strlen(id) + 1);
	node->alias = (char*)malloc(strlen(alias) + 1);

	strcpy(node->id, id);
	strcpy(node->alias, alias);

	return node;
}

LinkNode* CreateLinkNode(const char* alias, const char* id) {
	printf("CreateLinkNode id: %s alias: %s\n", id, alias);
	LinkNode* node = (LinkNode*)malloc(sizeof(LinkNode));

	node->id = (char*)malloc(strlen(id) + 1);
	node->alias = (char*)malloc(strlen(alias) + 1);

	strcpy(node->id, id);
	strcpy(node->alias, alias);

	return node;
}

FilterNode* CreateStringFilterNode(const char* alias, const char* property, Operator op, const char* value) {
	printf("CreateStringFilterNode alias: %s property: %s value: %s\n", alias, property, value);
	FilterNode* node = (FilterNode*)malloc(sizeof(FilterNode));

	node->alias = (char*)malloc(strlen(alias) + 1);
	node->property = (char*)malloc(strlen(property) + 1);
	node->strValue = (char*)malloc(strlen(value) + 1);

	strcpy(node->alias, alias);
	strcpy(node->property, property);
	strcpy(node->strValue, value);

	node->op = op;
	node->t = STRING;
	return node;
}

FilterNode* CreateNumericFilterNode(const char* alias, const char* property, Operator op, int value) {
	printf("CreateNumericFilterNode alias: %s property: %s value: %d\n", alias, property, value);
	FilterNode* node = (FilterNode*)malloc(sizeof(FilterNode));

	node->alias = (char*)malloc(strlen(alias) + 1);
	node->property = (char*)malloc(strlen(property) + 1);

	strcpy(node->property, property);
	strcpy(node->alias, alias);

	node->op = op;
	node->t = NUMERIC;
	return node;
}

MatchNode* CreateMatchNode(RelationshipNode* relationshipNode) {
	MatchNode* matchNode = (MatchNode*)malloc(sizeof(MatchNode));
	matchNode->relationshipNode = relationshipNode;

	return matchNode;
}

WhereNode* CreateWhereNode(Vector* filters) {
	WhereNode* whereNode = (WhereNode*)malloc(sizeof(WhereNode));
	whereNode->filters = filters;
	return whereNode;
}

VariableNode* CreateVariableNode(const char* alias, const char* property) {
	VariableNode* variableNode = (VariableNode*)malloc(sizeof(VariableNode));
	variableNode->alias = (char*)malloc(strlen(alias) + 1);
	variableNode->property = (char*)malloc(strlen(property) + 1);
	
	strcpy(variableNode->alias, alias);
	strcpy(variableNode->property, property);
	return variableNode;
}

ReturnNode* CreateReturnNode(Vector* variables) {
	ReturnNode* returnNode = malloc(sizeof(ReturnNode));
	returnNode->variables = variables;

	return returnNode;
}

RelationshipNode* CreateRelationshipNode(ItemNode* src, LinkNode* relation, ItemNode* dest) {
	printf("CreateRelationshipNode src: %p relation: %p dest: %p \n", src, relation, dest);

	RelationshipNode* relationshipNode = (RelationshipNode*)malloc(sizeof(RelationshipNode));
	relationshipNode->src = src;
	relationshipNode->relation = relation;
	relationshipNode->dest = dest;

	return relationshipNode;
}

QueryExpressionNode* queryExpressionNode;

QueryExpressionNode* CreateQueryExpressionNode(MatchNode* matchNode, WhereNode* whereNode, ReturnNode* returnNode) {
	queryExpressionNode = (QueryExpressionNode*)malloc(sizeof(QueryExpressionNode));
	
	queryExpressionNode->matchNode = matchNode;
	queryExpressionNode->whereNode = whereNode;
	queryExpressionNode->returnNode = returnNode;

	return queryExpressionNode;
}

void FreeItemNode(ItemNode* itemNode) {
	free(itemNode->id);
	free(itemNode->alias);
	free(itemNode);
}

void FreeLinkNode(LinkNode* linkNode) {
	free(linkNode->id);
	free(linkNode->alias);
	free(linkNode);
}

void FreeFilterNode(FilterNode* filterNode) {	
	free(filterNode->alias);
	free(filterNode->property);

	if(filterNode->t == STRING) {
		free(filterNode->strValue);
	}
}

void FreeRelationshipNode(RelationshipNode* relationshipNode) {
	FreeItemNode(relationshipNode->src);
	FreeItemNode(relationshipNode->dest);
	FreeLinkNode(relationshipNode->relation);
	free(relationshipNode);
}

void FreeMatchNode(MatchNode* matchNode) {
	FreeRelationshipNode(matchNode->relationshipNode);
	free(matchNode);
}

void FreeWhereNode(WhereNode* whereNode) {
	for (int i = 0; i < Vector_Size(whereNode->filters); i++) {
		FilterNode *node;
		Vector_Get(whereNode->filters, i, &node);
		FreeFilterNode(node);
	}

	Vector_Free(whereNode->filters);
	free(whereNode);
}

void FreeVariableNode(VariableNode* variableNode) {
	free(variableNode->alias);
	free(variableNode->property);
	free(variableNode);
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

void FreeQueryExpressionNode(QueryExpressionNode* queryExpressionNode) {
	FreeMatchNode(queryExpressionNode->matchNode);
	FreeWhereNode(queryExpressionNode->whereNode);
	FreeReturnNode(queryExpressionNode->returnNode);
}