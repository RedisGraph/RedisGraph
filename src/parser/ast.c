#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../graph/graph_entity.h"

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

AST_LinkEntity* New_AST_LinkEntity(char *alias, char *label, Vector *properties, AST_LinkDirection dir) {
	AST_LinkEntity* le = (AST_LinkEntity*)calloc(1, sizeof(AST_LinkEntity));
	le->direction = dir;
	le->ge.t = N_LINK;
	le->ge.properties = properties;
	
	if(label != NULL) {
		le->ge.label = strdup(label);
	}
	if(alias != NULL) {
		le->ge.alias = strdup(alias);
	}

	return le;
}

AST_NodeEntity* New_AST_NodeEntity(char *alias, char *label, Vector *properties) {
	AST_NodeEntity* ne = (AST_NodeEntity*)calloc(1, sizeof(AST_NodeEntity));
	ne->t = N_ENTITY;
	ne->properties = properties;
	
	if(alias != NULL) {
		ne->alias = strdup(alias);
	}
	if(label != NULL) {
		ne->label = strdup(label);
	}

	return ne;
}

void Free_AST_GraphEntity(AST_GraphEntity *graphEntity) {
	if(graphEntity->label != NULL) {
		free(graphEntity->label);
	}
	if(graphEntity->alias != NULL) {
		free(graphEntity->alias);
	}
	if(graphEntity->properties != NULL) {
		for(int i = 0; i < Vector_Size(graphEntity->properties); i++) {
			SIValue *val;
			Vector_Get(graphEntity->properties, i, &val);
			SIValue_Free(val);
			free(val);
		}
		Vector_Free(graphEntity->properties);
	}
	free(graphEntity);
}

AST_MatchNode* New_AST_MatchNode(Vector *elements) {
	AST_MatchNode *matchNode = (AST_MatchNode*)malloc(sizeof(AST_MatchNode));
	matchNode->graphEntities = elements;

	return matchNode;
}

void Free_AST_MatchNode(AST_MatchNode *matchNode) {
	for(int i = 0; i < Vector_Size(matchNode->graphEntities); i++) {
		GraphEntity *ge;
		Vector_Get(matchNode->graphEntities, i, &ge);
		FreeGraphEntity(ge);
	}

	Vector_Free(matchNode->graphEntities);
	free(matchNode);
}

AST_CreateNode* New_AST_CreateNode(Vector *elements) {
	AST_CreateNode *createNode = (AST_CreateNode*)malloc(sizeof(AST_CreateNode));
	createNode->graphEntities = NewVector(AST_GraphEntity *, 1);

	for(int i = 0; i < Vector_Size(elements); i++) {
		Vector *v;
		Vector_Get(elements, i, &v);

		for(int j = 0; j < Vector_Size(v); j++) {
			AST_GraphEntity *ge;
			Vector_Get(v, j, &ge);
			Vector_Push(createNode->graphEntities, ge);
		}
		
		Vector_Free(v);
	}

	return createNode;
}

void Free_AST_CreateNode(AST_CreateNode *createNode) {
	if(!createNode) return;
	
	for(int i = 0; i < Vector_Size(createNode->graphEntities); i++) {
		GraphEntity *ge;
		Vector_Get(createNode->graphEntities, i, &ge);
		FreeGraphEntity(ge);
	}

	Vector_Free(createNode->graphEntities);
	free(createNode);
}

AST_WhereNode* New_AST_WhereNode(AST_FilterNode *filters) {
	AST_WhereNode *whereNode = (AST_WhereNode*)malloc(sizeof(AST_WhereNode));
	whereNode->filters = filters;
	return whereNode;
}

void Free_AST_WhereNode(AST_WhereNode *whereNode) {
	if(!whereNode) return;

	Free_AST_FilterNode(whereNode->filters);
	free(whereNode);
}


AST_ReturnNode* New_AST_ReturnNode(Vector *returnElements, int distinct) {
	AST_ReturnNode *returnNode = (AST_ReturnNode*)malloc(sizeof(AST_ReturnNode));
	returnNode->returnElements = returnElements;
	returnNode->distinct = distinct;
	return returnNode;
}

void Free_AST_ReturnNode(AST_ReturnNode *returnNode) {
	for (int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
		AST_ReturnElementNode *node;
		Vector_Get(returnNode->returnElements, i, &node);
		Free_AST_ReturnElementNode(node);
	}
	
	Vector_Free(returnNode->returnElements);
	free(returnNode);
}

AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ReturnElementType type, AST_Variable* variable, const char* aggFunc, const char* alias) {
	AST_ReturnElementNode *returnElementNode = (AST_ReturnElementNode*)malloc(sizeof(AST_ReturnElementNode));
	returnElementNode->type = type;
	returnElementNode->variable = variable;
	returnElementNode->func = NULL;
	returnElementNode->alias = NULL;

	if(type == N_AGG_FUNC) {
		returnElementNode->func = strdup(aggFunc);
	}

	if(alias != NULL) {
		returnElementNode->alias = strdup(alias);
	}
	
	return returnElementNode;
}

void Free_AST_ReturnElementNode(AST_ReturnElementNode *returnElementNode) {
	if(returnElementNode != NULL) {
		Free_AST_Variable(returnElementNode->variable);

		if(returnElementNode->type == N_AGG_FUNC) {
			free(returnElementNode->func);
		}
		if(returnElementNode->alias != NULL) {
			free(returnElementNode->alias);
		}

		free(returnElementNode);
	}
}


AST_QueryExpressionNode* New_AST_QueryExpressionNode(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
												     AST_CreateNode *createNode, AST_ReturnNode *returnNode,
													 AST_OrderNode *orderNode, AST_LimitNode *limitNode) {
	AST_QueryExpressionNode *queryExpressionNode = (AST_QueryExpressionNode*)malloc(sizeof(AST_QueryExpressionNode));
	
	queryExpressionNode->matchNode = matchNode;
	queryExpressionNode->whereNode = whereNode;
	queryExpressionNode->createNode = createNode;
	queryExpressionNode->returnNode = returnNode;
	queryExpressionNode->orderNode = orderNode;
	queryExpressionNode->limitNode = limitNode;

	return queryExpressionNode;
}

void Free_AST_QueryExpressionNode(AST_QueryExpressionNode *queryExpressionNode) {
	Free_AST_MatchNode(queryExpressionNode->matchNode);
	Free_AST_CreateNode(queryExpressionNode->createNode);
	Free_AST_WhereNode(queryExpressionNode->whereNode);
	Free_AST_ReturnNode(queryExpressionNode->returnNode);
	Free_AST_OrderNode(queryExpressionNode->orderNode);
	free(queryExpressionNode);
}

AST_Variable* New_AST_Variable(const char* alias, const char* property) {
	AST_Variable *v = (AST_Variable*)calloc(1, sizeof(AST_Variable));

	if(alias != NULL) {
		v->alias = strdup(alias);
	}
	if(property != NULL) {
		v->property = strdup(property);
	}

	return v;
}

void Free_AST_Variable(AST_Variable *v) {
	if(v != NULL) {
		if(v->alias != NULL) {
			free(v->alias);
		}
		if(v->property != NULL) {
			free(v->property);
		}
		free(v);
	}
}

AST_OrderNode* New_AST_OrderNode(Vector *columns, AST_OrderByDirection direction) {
	AST_OrderNode *orderNode = (AST_OrderNode*)malloc(sizeof(AST_OrderNode));
	orderNode->columns = columns;
	orderNode->direction = direction;
	return orderNode;
}

void Free_AST_OrderNode(AST_OrderNode *orderNode) {
	if(orderNode != NULL) {
		for(int i = 0; i < Vector_Size(orderNode->columns); i++) {
			AST_ColumnNode *c = NULL;
			Vector_Get(orderNode->columns, i , &c);
			Free_AST_ColumnNode(c);
		}

		Vector_Free(orderNode->columns);
		free(orderNode);
	}
}

AST_ColumnNode* New_AST_ColumnNode(const char *alias, const char *property, AST_ColumnNodeType type) {
	AST_ColumnNode *node = malloc(sizeof(AST_ColumnNode));
	node->type = type;
	node->alias = NULL;
	node->property = NULL;

	node->alias = malloc(sizeof(char) * (strlen(alias) + 1));
	strcpy(node->alias, alias);

	if(type == N_VARIABLE) {
		node->property = malloc(sizeof(char) * (strlen(property) + 1));
		strcpy(node->property, property);
	}

	return node;
}

AST_ColumnNode* AST_ColumnNodeFromVariable(const AST_Variable *variable) {
	return New_AST_ColumnNode(variable->alias, variable->property, N_VARIABLE);
}

AST_ColumnNode* AST_ColumnNodeFromAlias(const char *alias) {
	return New_AST_ColumnNode(alias, NULL, N_ALIAS);
}

void Free_AST_ColumnNode(AST_ColumnNode* node) {
	if(node != NULL) {
		if(node->alias != NULL) {
			free(node->alias);
		}
		if(node->type == N_VARIABLE && node->property != NULL) {
			free(node->property);
		}
		free(node);
	}
}

AST_LimitNode* New_AST_LimitNode(int limit) {
	AST_LimitNode* limitNode = (AST_LimitNode*)malloc(sizeof(AST_LimitNode));
	limitNode->limit = limit;
	return limitNode;
}

void Free_AST_LimitNode(AST_LimitNode* limitNode) {
	if(limitNode) {
		free(limitNode);
	}
}
