/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __ARITHMETIC_EXPRESSION_H
#define __ARITHMETIC_EXPRESSION_H

#include "../graph/graph_entity.h"
#include "../graph/query_graph.h"
#include "../parser/ast.h"
#include "./agg_ctx.h"

/* Forward declarations. */
struct AR_ExpNode;

/* AR_ExpNodeType lists the type of nodes within
 * an arithmetic expression tree. */
typedef enum {
    AR_EXP_OP,
    AR_EXP_OPERAND,
} AR_ExpNodeType;

/* AR_OPType type of operation 
 * either an aggregation function which requires a context
 * or a stateless function. */
typedef enum {
    AR_OP_AGGREGATE,
    AR_OP_FUNC,
} AR_OPType;

/* AR_OperandNodeType type of leaf node,
 * either a constant: 3, or a variable: node.property. */
typedef enum {
    AR_EXP_CONSTANT,
    AR_EXP_VARIADIC,
} AR_OperandNodeType;

/* AR_Func - Function pointer to an operation with an arithmetic expression */
typedef SIValue (*AR_Func)(SIValue *argv, int argc);

/* Mathematical functions - numeric */
SIValue AR_ADD(SIValue *argv, int argc);   /* returns the summation of given values. */
SIValue AR_SUB(SIValue *argv, int argc);   /* returns the subtracting given values. */
SIValue AR_MUL(SIValue *argv, int argc);   /* returns the multiplication of given values. */
SIValue AR_DIV(SIValue *argv, int argc);   /* returns the divition of given values. */
SIValue AR_ABS(SIValue *argv, int argc);   /* returns the absolute value of the given number. */
SIValue AR_CEIL(SIValue *argv, int argc);  /* returns the smallest floating point number that is greater than or equal to the given number and equal to a mathematical integer. */
SIValue AR_FLOOR(SIValue *argv, int argc); /* returns the largest floating point number that is less than or equal to the given number and equal to a mathematical integer. */
SIValue AR_RAND(SIValue *argv, int argc);  /* returns a random floating point number in the range from 0 to 1; i.e. [0,1]. The numbers returned follow an approximate uniform distribution. */
SIValue AR_ROUND(SIValue *argv, int argc); /* returns the value of the given number rounded to the nearest integer. */
SIValue AR_SIGN(SIValue *argv, int argc);  /* returns the signum of the given number: 0 if the number is 0, -1 for any negative number, and 1 for any positive number. */

/* String functions */
SIValue AR_LEFT(SIValue *argv, int argc);      /* returns a string containing the specified number of leftmost characters of the original string. */
SIValue AR_LTRIM(SIValue *argv, int argc);     /* returns the original string with leading whitespace removed. */
SIValue AR_REPLACE(SIValue *argv, int argc);   /* returns a string in which all occurrences of a specified string in the original string have been replaced by ANOTHER (specified) string. */
SIValue AR_REVERSE(SIValue *argv, int argc);   /* returns a string in which the order of all characters in the original string have been reversed. */
SIValue AR_RIGHT(SIValue *argv, int argc);     /* returns a string containing the specified number of rightmost characters of the original string. */
SIValue AR_RTRIM(SIValue *argv, int argc);     /* returns the original string with trailing whitespace removed. */
SIValue AR_SPLIT(SIValue *argv, int argc);     /* returns a list of strings resulting from the splitting of the original string around matches of the given delimiter. */
SIValue AR_SUBSTRING(SIValue *argv, int argc); /* returns a substring of the original string, beginning with a 0-based index start and length. */
SIValue AR_TOLOWER(SIValue *argv, int argc);   /* returns the original string in lowercase. */
SIValue AR_TOSTRING(SIValue *argv, int argc);  /* converts an integer, float or boolean value to a string. */
SIValue AR_TOUPPER(SIValue *argv, int argc);   /* returns the original string in uppercase. */
SIValue AR_TRIM(SIValue *argv, int argc);      /* returns the original string with leading and trailing whitespace removed. */
SIValue AR_CONCAT(SIValue *argv, int argc);    /* returns a string concatenation of given values. */

SIValue AR_ID(SIValue *argv, int argc);        /* returns the id of a relationship or node. */

void AR_RegisterFuncs();                       /* Registers all arithmetic functions. */
AR_Func AR_GetFunc(char *func_name);           /* Get arithmetic function. */
bool AR_FuncExists(const char *func_name);     /* Check to see if function exists. */

/* Register an arithmetic function. */
void AR_RegFunc(char *func_name, size_t func_name_len, AR_Func func);

/* Op represents an operation applied to child args. */
typedef struct {
    union {
        AR_Func f;
        AggCtx *agg_func;
    };                              /* Operation to perform on children. */
    char *func_name;                /* Name of function. */
    int child_count;                /* Number of children. */
    struct AR_ExpNode **children;   /* Child nodes. */
    AR_OPType type;
} AR_OpNode;

/* OperandNode represents either a constant numeric value, 
 * or a graph entity property. */
typedef struct {
    union {
        SIValue constant;
        struct {
            GraphEntity **entity;
            char *entity_alias;
			char *entity_prop;
		} variadic;
    };
    AR_OperandNodeType type;
} AR_OperandNode;

/* AR_ExpNode a node within an arithmetic expression tree, 
 * This node can take one of two forms:
 * 1. OpNode
 * 2. OperandNode */
struct AR_ExpNode {
    union {
        AR_OperandNode operand;
        AR_OpNode op;
    };
    AR_ExpNodeType type;
};

typedef struct AR_ExpNode AR_ExpNode;

/* Return AR_OperandNodeType for operands and -1 for operations. */
int AR_EXP_GetOperandType(AR_ExpNode *exp);

/* Evaluate arithmetic expression tree. */
SIValue AR_EXP_Evaluate(const AR_ExpNode *root);
void AR_EXP_Aggregate(const AR_ExpNode *root);
void AR_EXP_Reduce(const AR_ExpNode *root);

/* Create arithmetic expression node. */
AR_ExpNode* AR_EXP_NewConstOperandNode(SIValue constant);
AR_ExpNode* AR_EXP_NewVariableOperandNode(GraphEntity **entity, const char *entity_prop, const char *entity_alias);
AR_ExpNode* AR_EXP_NewOpNode(char *func_name, int child_count);

/* Utility functions */
/* Traverse an expression tree and add all graph entity aliases
 * (from variadics) to a triemap. */
void AR_EXP_CollectAliases(AR_ExpNode *root, TrieMap *aliases);

/* Search for an aggregation node within the expression tree.
 * Return 1 and sets agg_node to the aggregation node if exists,
 * Please note an expression tree can only contain a single aggregation node. */
int AR_EXP_ContainsAggregation(AR_ExpNode *root, AR_ExpNode **agg_node);

/* Constructs string representation of arithmetic expression tree. */
void AR_EXP_ToString(const AR_ExpNode *root, char **str);

/* Construct an arithmetic expression tree from ast arithmetic expression node. */
AR_ExpNode* AR_EXP_BuildFromAST(const AST_ArithmeticExpressionNode *exp, const QueryGraph *g);

/* Free arithmetic expression tree. */
void AR_EXP_Free(AR_ExpNode *root);

#endif