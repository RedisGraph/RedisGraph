/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "ast_arithmetic_expression.h"
#include "../util/rmalloc.h"
#include "../arithmetic/repository.h"
#include "../arithmetic/arithmetic_expression.h"

AST_ArithmeticExpressionNode* New_AST_AR_EXP_VariableOperandNode(char* alias, char *property) {
	AST_ArithmeticExpressionNode *node = rm_malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OPERAND;
	node->operand.type = AST_AR_EXP_VARIADIC;
	node->operand.variadic.alias = rm_strdup(alias);
	if(property) {
		// This is a collapsed entity.
		node->operand.variadic.property = rm_strdup(property);
	} else {
		node->operand.variadic.property = NULL;
	}
	return node;
}

AST_ArithmeticExpressionNode* New_AST_AR_EXP_ConstOperandNode(SIValue constant) {
	AST_ArithmeticExpressionNode *node = rm_malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OPERAND;
	node->operand.type = AST_AR_EXP_CONSTANT;
	node->operand.constant = constant;
	return node;
}

AST_ArithmeticExpressionNode* New_AST_AR_EXP_OpNode(char *func, Vector *args) {
	AST_ArithmeticExpressionNode *node = rm_malloc(sizeof(AST_ArithmeticExpressionNode));
	node->type = AST_AR_EXP_OP;
	node->op.function = func;
	node->op.args = args;
	return node;
}

void AST_AR_EXP_GetAliases(const AST_ArithmeticExpressionNode *exp, TrieMap *aliases) {
	if (exp->type == AST_AR_EXP_OP) {
		/* Process operands. */
		for (int i = 0; i < Vector_Size(exp->op.args); i++) {
			AST_ArithmeticExpressionNode *child;
			Vector_Get(exp->op.args, i, &child);
			AST_AR_EXP_GetAliases(child, aliases);
		}
	} else {
		/* Check specific operand */
		if (exp->operand.type == AST_AR_EXP_VARIADIC) {
			char *alias = exp->operand.variadic.alias;
			if (alias) TrieMap_Add(aliases, alias, strlen(alias), NULL, TrieMap_DONT_CARE_REPLACE);
    }
  }
}

void AST_AR_EXP_GetFunctions(const AST_ArithmeticExpressionNode *exp, TrieMap *functions) {
	if(exp->type == AST_AR_EXP_OP) {
		AST_ArithmeticExpressionOP op = exp->op;
		TrieMap_Add(functions, op.function, strlen(op.function), NULL, TrieMap_DONT_CARE_REPLACE);
		for(int i = 0; i < Vector_Size(op.args); i++) {
			AST_ArithmeticExpressionNode *arg;
			Vector_Get(op.args, i, &arg);
			AST_AR_EXP_GetFunctions(arg, functions);
		}
	}
}

int AST_AR_EXP_ContainsAggregation(const AST_ArithmeticExpressionNode *exp) {
    if(exp->type == AST_AR_EXP_OPERAND) {
        return 0;
    }

    /* Try to get an aggregation function. */
    if(Agg_FuncExists(exp->op.function)) return 1;

    /* Scan sub expressions. */
    for(int i = 0; i < Vector_Size(exp->op.args); i++) {
			AST_ArithmeticExpressionNode *child_exp;
			Vector_Get(exp->op.args, i, &child_exp);
			if(child_exp->type != AST_AR_EXP_OP) continue;
			if(AST_AR_EXP_ContainsAggregation(child_exp)) return 1;
    }

    return 0;
}

void Free_AST_ArithmeticExpressionNode(AST_ArithmeticExpressionNode *arExpNode) {
	if(!arExpNode) return;
	/* Free arithmetic expression operation. */
	if(arExpNode->type == AST_AR_EXP_OP) {
		/* free each argument. */
		for(int i = 0; i < Vector_Size(arExpNode->op.args); i++) {
			AST_ArithmeticExpressionNode *child;
			Vector_Get(arExpNode->op.args, i, &child);
			Free_AST_ArithmeticExpressionNode(child);
		}
		Vector_Free(arExpNode->op.args);
	} else {
		/* Node is an arithmetic expression operand. */
		if(arExpNode->operand.type == AST_AR_EXP_VARIADIC) {
			rm_free(arExpNode->operand.variadic.alias);
			rm_free(arExpNode->operand.variadic.property);
		}
	}
	/* Finally we can free the node. */
	rm_free(arExpNode);
}
