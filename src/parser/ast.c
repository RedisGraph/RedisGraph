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

AST_SetElement* New_AST_SetElement(AST_Variable *updated_entity, AST_ArithmeticExpressionNode *exp) {
	AST_SetElement *set_element = malloc(sizeof(AST_SetElement));
	set_element->entity = updated_entity;
	set_element->exp = exp;
	return set_element;
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
	AST_CreateNode *create_node = (AST_CreateNode*)malloc(sizeof(AST_CreateNode));
	create_node->graphEntities = elements;
	return create_node;
}

AST_SetNode* New_AST_SetNode(Vector *elements) {
	AST_SetNode *set_node = (AST_SetNode*)malloc(sizeof(AST_SetNode));
	set_node->set_elements = elements;
	return set_node;
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

AST_DeleteNode* New_AST_DeleteNode(Vector *elements) {
	AST_DeleteNode *deleteNode = (AST_DeleteNode*)malloc(sizeof(AST_DeleteNode));
	deleteNode->graphEntities = elements;
	return deleteNode;
}

void Free_AST_DeleteNode(AST_DeleteNode *deleteNode) {
	if(!deleteNode)	return;
	Vector_Free(deleteNode->graphEntities);
	free(deleteNode);
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

AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ArithmeticExpressionNode *exp, const char* alias) {
	AST_ReturnElementNode *returnElementNode = (AST_ReturnElementNode*)malloc(sizeof(AST_ReturnElementNode));
	returnElementNode->exp = exp;
	returnElementNode->alias = NULL;

	if(alias != NULL) returnElementNode->alias = strdup(alias);
	
	return returnElementNode;
}

void Free_AST_ReturnElementNode(AST_ReturnElementNode *returnElementNode) {
	if(returnElementNode != NULL) {
		Free_AST_ArithmeticExpressionNode(returnElementNode->exp);

		if(returnElementNode->alias != NULL) {
			free(returnElementNode->alias);
		}

		free(returnElementNode);
	}
}

AST_QueryExpressionNode* New_AST_QueryExpressionNode(AST_MatchNode *matchNode, AST_WhereNode *whereNode,
												     AST_CreateNode *createNode, AST_SetNode *setNode,
													 AST_DeleteNode *deleteNode, AST_ReturnNode *returnNode,
													 AST_OrderNode *orderNode, AST_LimitNode *limitNode) {
	AST_QueryExpressionNode *queryExpressionNode = (AST_QueryExpressionNode*)malloc(sizeof(AST_QueryExpressionNode));
	
	queryExpressionNode->matchNode = matchNode;
	queryExpressionNode->whereNode = whereNode;
	queryExpressionNode->createNode = createNode;
	queryExpressionNode->setNode = setNode;
	queryExpressionNode->deleteNode = deleteNode;
	queryExpressionNode->returnNode = returnNode;
	queryExpressionNode->orderNode = orderNode;
	queryExpressionNode->limitNode = limitNode;

	return queryExpressionNode;
}

void Free_AST_QueryExpressionNode(AST_QueryExpressionNode *queryExpressionNode) {
	Free_AST_MatchNode(queryExpressionNode->matchNode);
	Free_AST_CreateNode(queryExpressionNode->createNode);
	Free_AST_DeleteNode(queryExpressionNode->deleteNode);
	Free_AST_WhereNode(queryExpressionNode->whereNode);
	Free_AST_ReturnNode(queryExpressionNode->returnNode);
	Free_AST_OrderNode(queryExpressionNode->orderNode);
	free(queryExpressionNode);
}

AST_Validation _Validate_MATCH_Clause(const AST_QueryExpressionNode* ast, char **reason) {
	return AST_VALID;
}

AST_Validation _Validate_WHERE_Clause(const AST_QueryExpressionNode* ast, char **reason) {
	return AST_VALID;
}

AST_Validation _Validate_CREATE_Clause(const AST_QueryExpressionNode* ast, char **reason) {
	return AST_VALID;
}

AST_Validation _Validate_SET_Clause(const AST_QueryExpressionNode* ast, char **reason) {
	char* undefined_alias;
	
	if (!ast->setNode) {
		return AST_VALID;
	}

	if (!ast->matchNode) {
		return AST_INVALID;
	}
	
    Vector *update_aliases = NewVector(char*, Vector_Size(ast->setNode->set_elements));
	for (int i=0; i < Vector_Size(ast->setNode->set_elements); i++) {
		AST_SetElement *update_entity = NULL;
        Vector_Get(ast->setNode->set_elements, i, &update_entity);
		Vector_Push(update_aliases, update_entity->entity->alias);
	}

	AST_Validation res = _Validate_Aliases_In_Match_Clause(
		update_aliases, ast->matchNode->graphEntities, &undefined_alias);
	Vector_Free(update_aliases);

	if (res != AST_VALID) {
		*reason = undefined_alias;
		return AST_INVALID;
	}

	return AST_VALID;
}

AST_Validation _Validate_DELETE_Clause(const AST_QueryExpressionNode* ast, char **reason) {
	char* undefined_alias;

	if (!ast->deleteNode) {
		return AST_VALID;
	}

	if (!ast->matchNode) {
		return AST_INVALID;
	}

	if (_Validate_Aliases_In_Match_Clause(ast->deleteNode->graphEntities, 
			ast->matchNode->graphEntities, &undefined_alias) != AST_VALID) {
		*reason = undefined_alias;
		return AST_INVALID;
	}

	return AST_VALID;
}

AST_Validation _Validate_RETURN_Clause(const AST_QueryExpressionNode* ast, char **reason) {
	char* undefined_alias;

	if (!ast->returnNode) {
		return AST_VALID;
	}

	if (!ast->matchNode) {
		return AST_INVALID;
	}

	Vector *return_aliases = NewVector(char*, Vector_Size(ast->returnNode->returnElements));
	for (int i=0; i < Vector_Size(ast->returnNode->returnElements); i++) {
		AST_ReturnElementNode *return_entity = NULL;
        Vector_Get(ast->returnNode->returnElements, i, &return_entity);
		_Get_All_Aliases_From_Expression(return_entity->exp, return_aliases);
	}

	AST_Validation res = _Validate_Aliases_In_Match_Clause(
		return_aliases, ast->matchNode->graphEntities, &undefined_alias);
	Vector_Free(return_aliases);
	
	if (res != AST_VALID) {
		*reason = undefined_alias;
		return AST_INVALID;
	}

	return AST_VALID;
}

AST_Validation _Validate_Aliases_In_Match_Clause(const Vector* aliasesToCheck, 
										         const Vector* matchAliases, 
										         char** undefined_alias) {
    /* Check that all the aliases that are in aliasesToCheck Vector exists in the match clause */

	for (int i = 0; i < Vector_Size(aliasesToCheck); i++) {
        char *alias_to_check = NULL;
        Vector_Get(aliasesToCheck, i, &alias_to_check);

		int exists = 0;
		for (int j = 0; j < Vector_Size(matchAliases); j++) {
			AST_GraphEntity *match_entity = NULL;
        	Vector_Get(matchAliases, j, &match_entity);
			if (strcmp(alias_to_check, match_entity->alias) == 0) {
				exists = 1;
				break;
			}
		}

		if (!exists) {
		    *undefined_alias = (char*)malloc(sizeof(char) * 
				(strlen(" not defined" ) + strlen(alias_to_check) + 1));
			sprintf(*undefined_alias, "%s not defined", alias_to_check);
			return AST_INVALID;
		}
    }

	return AST_VALID;
}

AST_Validation Validate_AST(const AST_QueryExpressionNode* ast, char **reason) {
	/* AST must include either a MATCH or CREATE clause. */
	if(ast->matchNode == NULL && ast->createNode == NULL) {
		*reason = "Query must specify either MATCH or CREATE clause.";
		return AST_INVALID;
	}

	if(_Validate_MATCH_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_WHERE_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_CREATE_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_SET_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_DELETE_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	if(_Validate_RETURN_Clause(ast, reason) != AST_VALID) {
		return AST_INVALID;
	}

	return AST_VALID;
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

AST_ArithmeticExpressionNode* New_AST_AR_EXP_ConstOperandNode(SIValue constant) {
	AST_ArithmeticExpressionNode *node = malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OPERAND;
	node->operand.type = AST_AR_EXP_CONSTANT;
	node->operand.constant = constant;
	return node;
}

AST_ArithmeticExpressionNode* New_AST_AR_EXP_VariableOperandNode(char* alias, char *property) {
	AST_ArithmeticExpressionNode *node = malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OPERAND;
	node->operand.type = AST_AR_EXP_VARIADIC;
	node->operand.variadic.alias = strdup(alias);
	if(property) {
		// This is a collapsed entity.
		node->operand.variadic.property = strdup(property);
	} else {
		node->operand.variadic.property = NULL;
	}
	return node;
}

AST_ArithmeticExpressionNode* New_AST_AR_EXP_OpNode(char *func, Vector *args) {
	AST_ArithmeticExpressionNode *node = malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OP;
	node->op.function = strdup(func);
	node->op.args = args;
	return node;
}

void Free_AST_ArithmeticExpressionNode(AST_ArithmeticExpressionNode *arExpNode) {
	/* Free arithmetic expression operation. */
	if(arExpNode->type == AST_AR_EXP_OP) {
		/* Free each argument. */
		for(int i = 0; i < Vector_Size(arExpNode->op.args); i++) {
			AST_ArithmeticExpressionNode *child;
			Vector_Get(arExpNode->op.args, i, &child);
			Free_AST_ArithmeticExpressionNode(child);
		}
		Vector_Free(arExpNode->op.args);
	} else {
		/* Node is an arithmetic expression operand. */
		if(arExpNode->operand.type == AST_AR_EXP_VARIADIC) {
			free(arExpNode->operand.variadic.alias);
			free(arExpNode->operand.variadic.property);
		}
	}
	/* Finaly we can free the node. */
	free(arExpNode);
}

void _Get_All_Aliases_From_Expression(const AST_ArithmeticExpressionNode *exp, Vector *aliases) {
    /* Find all the aliases in an expression */

    if(exp->type == AST_AR_EXP_OP) {
        /* Process operands. */
        for(int i = 0; i < Vector_Size(exp->op.args); i++) {
            AST_ArithmeticExpressionNode *child;
            Vector_Get(exp->op.args, i, &child);
            _Get_All_Aliases_From_Expression(child, aliases);
        }
    } else {
		/* Check specific operand */
        if(exp->operand.type == AST_AR_EXP_VARIADIC) {
            Vector_Push(aliases, (void*)exp->operand.variadic.alias);
        }
    }
}
