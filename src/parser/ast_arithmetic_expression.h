/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef _AST_ARITHMETIC_EXPRESSION_H
#define _AST_ARITHMETIC_EXPRESSION_H

#include "../util/vector.h"
#include "../util/triemap/triemap.h"
#include "../value.h"

/* If an AST_ArithmeticExpression_Node is a variadic with an entity alias and no
 * property, it may refer to a full graph entity (or an aliased constant,
 * such as an UNWIND list). */
#define EXPRESSION_IS_ALIAS(a) (a)->type == AST_AR_EXP_OPERAND && \
                               (a)->operand.type == AST_AR_EXP_VARIADIC && \
                               (a)->operand.variadic.property == NULL

/* ArExpNodeType lists the type of nodes within
 * an arithmetic expression tree. */
typedef enum {
    AST_AR_EXP_OP,
    AST_AR_EXP_OPERAND,
} AST_ArithmeticExpression_NodeType;

typedef enum {
    AST_AR_EXP_CONSTANT,
    AST_AR_EXP_VARIADIC,
} AST_ArithmeticExpression_OperandNodeType;

typedef struct {
    char *function; /* Name of operation. */
    Vector *args;	/* Vector of AST_ArithmeticExpressionNode pointers. */
} AST_ArithmeticExpressionOP;

/* OperandNode represents either a constant numeric value, 
 * or a graph entity property. */
typedef struct {
    union {
        SIValue constant;
        struct {
			char *alias;
			char *property;
		} variadic;
    };
    AST_ArithmeticExpression_OperandNodeType type;
} AST_ArithmeticExpressionOperand;

typedef struct {
	union {
        AST_ArithmeticExpressionOperand operand;
        AST_ArithmeticExpressionOP op;
    };
    AST_ArithmeticExpression_NodeType type;
} AST_ArithmeticExpressionNode;

AST_ArithmeticExpressionNode* New_AST_AR_EXP_VariableOperandNode(char* alias, char *property);
AST_ArithmeticExpressionNode* New_AST_AR_EXP_ConstOperandNode(SIValue constant);
AST_ArithmeticExpressionNode* New_AST_AR_EXP_OpNode(char *func, Vector *args);

/* Find all the aliases in expression */
void AST_AR_EXP_GetAliases(const AST_ArithmeticExpressionNode *exp, TrieMap *aliases);

/* Find all functions in expression */
void AST_AR_EXP_GetFunctions(const AST_ArithmeticExpressionNode *exp, TrieMap *functions);

int AST_AR_EXP_ContainsAggregation(const AST_ArithmeticExpressionNode *exp);

void Free_AST_ArithmeticExpressionNode(AST_ArithmeticExpressionNode *arExpNode);

#endif
