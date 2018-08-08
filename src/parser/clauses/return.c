/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "./return.h"
#include "../../arithmetic/repository.h"

AST_ReturnElementNode* New_AST_ReturnElementNode(AST_ArithmeticExpressionNode *exp, const char* alias) {
	AST_ReturnElementNode *returnElementNode = (AST_ReturnElementNode*)malloc(sizeof(AST_ReturnElementNode));
	returnElementNode->exp = exp;
	returnElementNode->alias = NULL;

	if(alias != NULL) returnElementNode->alias = strdup(alias);
	
	return returnElementNode;
}

AST_ReturnNode* New_AST_ReturnNode(Vector *returnElements, int distinct) {
	AST_ReturnNode *returnNode = (AST_ReturnNode*)malloc(sizeof(AST_ReturnNode));
	returnNode->returnElements = returnElements;
	returnNode->distinct = distinct;
	return returnNode;
}

int ReturnClause_ContainsCollapsedNodes(const AST_ReturnNode *return_node) {
    if(!return_node) return 0;
    for(int i = 0; i < Vector_Size(return_node->returnElements); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(return_node->returnElements, i, &ret_elem);
        AST_ArithmeticExpressionNode *exp = ret_elem->exp;
        /* Detect collapsed entity,
         * A collapsed entity is represented by an arithmetic expression
         * of AST_AR_EXP_OPERAND type, 
         * The operand type should be AST_AR_EXP_VARIADIC,
         * lastly property should be missing. */
        if(exp->type == AST_AR_EXP_OPERAND &&
            exp->operand.type == AST_AR_EXP_VARIADIC &&
            exp->operand.variadic.property == NULL) {
                return 1;
        }
    }
    return 0;
}

int _ContainsAggregation(AST_ArithmeticExpressionNode *exp) {
    if(exp->type == AST_AR_EXP_OPERAND) {
        return 0;
    }
    
    /* Try to get an aggregation function. */
    AggCtx* ctx;
    Agg_GetFunc(exp->op.function, &ctx);
    if(ctx != NULL) return 1;

    /* Scan sub expressions. */
    for(int i = 0; i < Vector_Size(exp->op.args); i++) {
        AST_ArithmeticExpressionNode *child_exp;
        Vector_Get(exp->op.args, i, &child_exp);
        if(_ContainsAggregation(child_exp)) return 1;
    }

    return 0;
}

/* Checks if return clause uses aggregation. */
int ReturnClause_ContainsAggregation(const AST_ReturnNode *return_node) {
    if(!return_node) return 0;

    for(int i = 0; i < Vector_Size(return_node->returnElements); i++) {
        AST_ReturnElementNode *ret_elem;
        Vector_Get(return_node->returnElements, i, &ret_elem);
        AST_ArithmeticExpressionNode *exp = ret_elem->exp;

        /* Scan expression for an aggregation function. */
        if (_ContainsAggregation(exp)) return 1;
    }

    return 0;
}

void ReturnClause_ReferredNodes(const AST_ReturnNode *return_node, TrieMap *referred_nodes) {
    if(!return_node) return;
    int return_element_count = Vector_Size(return_node->returnElements);

    for(int i = 0; i < return_element_count; i++) {
        AST_ReturnElementNode *return_element;
        Vector_Get(return_node->returnElements, i, &return_element);
        
        AST_ArithmeticExpressionNode *exp = return_element->exp;
        AR_EXP_GetAliases(exp, referred_nodes);
    }
}

void ReturnClause_ReferredFunctions(const AST_ReturnNode *return_node, TrieMap *referred_funcs) {
    if(!return_node) return;
    int return_element_count = Vector_Size(return_node->returnElements);

    for(int i = 0; i < return_element_count; i++) {
        AST_ReturnElementNode *node;
        Vector_Get(return_node->returnElements, i, &node);
        
        AST_ArithmeticExpressionNode *exp = node->exp;
        AR_EXP_GetFunctions(exp, referred_funcs);
  }
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

void Free_AST_ReturnNode(AST_ReturnNode *returnNode) {
    if(! returnNode) return;
	for (int i = 0; i < Vector_Size(returnNode->returnElements); i++) {
		AST_ReturnElementNode *node;
		Vector_Get(returnNode->returnElements, i, &node);
		Free_AST_ReturnElementNode(node);
	}
	
	Vector_Free(returnNode->returnElements);
	free(returnNode);
}
