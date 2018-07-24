/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "ast_arithmetic_expression.h"

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

AST_ArithmeticExpressionNode* New_AST_AR_EXP_ConstOperandNode(SIValue constant) {
	AST_ArithmeticExpressionNode *node = malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OPERAND;
	node->operand.type = AST_AR_EXP_CONSTANT;
	node->operand.constant = constant;
	return node;
}

AST_ArithmeticExpressionNode* New_AST_AR_EXP_OpNode(char *func, Vector *args) {
	AST_ArithmeticExpressionNode *node = malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OP;
	node->op.function = strdup(func);
	node->op.args = args;
	return node;
}

void AR_EXP_GetAllAliases(const AST_ArithmeticExpressionNode *exp, TrieMap *aliases) {
	if (exp->type == AST_AR_EXP_OP) {
    /* Process operands. */
    for (int i = 0; i < Vector_Size(exp->op.args); i++) {
      AST_ArithmeticExpressionNode *child;
      Vector_Get(exp->op.args, i, &child);
      AR_EXP_GetAllAliases(child, aliases);
    }
  } else {
    /* Check specific operand */
    if (exp->operand.type == AST_AR_EXP_VARIADIC) {
			char *alias = exp->operand.variadic.alias;
			TrieMap_Add(aliases, alias, strlen(alias), NULL, NULL);
    }
  }
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
