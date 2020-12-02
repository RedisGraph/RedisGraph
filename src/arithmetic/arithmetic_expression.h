/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include <stddef.h>
#include "rax.h"
#include "./func_desc.h"
#include "../../deps/rax/rax.h"
#include "../execution_plan/record.h"
#include "../graph/entities/graph_entity.h"

/* AR_ExpNodeType lists the type of nodes within
 * an arithmetic expression tree. */
typedef enum {
	AR_EXP_UNKNOWN,
	AR_EXP_OP,
	AR_EXP_OPERAND
} AR_ExpNodeType;

/* AR_OperandNodeType - Type of an expression tree's leaf node. */
typedef enum {
	AR_EXP_OP_UNKNOWN,     // Should not occur.
	AR_EXP_CONSTANT,       // A constant, e.g. 3
	AR_EXP_VARIADIC,       // A variable, e.g. n
	AR_EXP_PARAM,          // A parameter, e.g. $p.
	AR_EXP_BORROW_RECORD   // A directive to store the current record.
} AR_OperandNodeType;

/* Success of an evaluation. */
typedef enum {
	EVAL_OK = 0,
	EVAL_ERR = (1 << 0),
	EVAL_FOUND_PARAM = (1 << 1),
} AR_EXP_Result;

/* Op represents an operation applied to child args. */
typedef struct {
	AR_FuncDesc *f;                 /* Operation to perform on children. */
	const char *func_name;          /* Name of function. */
	int child_count;                /* Number of children. */
	struct AR_ExpNode **children;   /* Child nodes. */
} AR_OpNode;

// OperandNode represents either constant, parameter, or graph entity
typedef struct {
	union {
		SIValue constant;
		const char *param_name;
		struct {
			const char *entity_alias;
			int entity_alias_idx;
		} variadic;
	};
	AR_OperandNodeType type;
} AR_OperandNode;

/* AR_ExpNode a node within an arithmetic expression tree,
 * This node can take one of two forms:
 * 1. OpNode
 * 2. OperandNode */
typedef struct AR_ExpNode {
	union {
		AR_OperandNode operand;
		AR_OpNode op;
	};
	AR_ExpNodeType type;
	// The string representation of the node, such as the literal string "ID(a) + 5"
	const char *resolved_name;
} AR_ExpNode;

/* Creates a new Arithmetic expression operation node */
AR_ExpNode *AR_EXP_NewOpNode(const char *func_name, uint child_count);

/* Creates a new Arithmetic expression variable operand node */
AR_ExpNode *AR_EXP_NewVariableOperandNode(const char *alias);

/* Creates a new Arithmetic expression extracting an attribute from an entity */
AR_ExpNode *AR_EXP_NewAttributeAccessNode(AR_ExpNode *entity, const char *attr);

/* Creates a new Arithmetic expression constant operand node */
AR_ExpNode *AR_EXP_NewConstOperandNode(SIValue constant);

/* Creates a new Arithmetic expression parameter operand node. */
AR_ExpNode *AR_EXP_NewParameterOperandNode(const char *param_name);

/* Creates a new Arithmetic expression that will resolve to the current Record. */
AR_ExpNode *AR_EXP_NewRecordNode(void);

/* Compact tree by evaluating all contained functions that can be resolved right now.
 * The function returns true if it managed to compact the expression.
 * The reduce_params flag indicates if parameters should be evaluated.
 * The val pointer is out-by-ref returned computation. */
bool AR_EXP_ReduceToScalar(AR_ExpNode *root, bool reduce_params, SIValue *val);

/* Evaluate arithmetic expression tree. */
SIValue AR_EXP_Evaluate(AR_ExpNode *root, const Record r);

/* Evaluate expression tree, retrieving final results from aggregate nodes. */
SIValue AR_EXP_AggregateGetResults(AR_ExpNode *root, const Record r);

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/* Traverse an expression tree and add all entity aliases to a rax. */
void AR_EXP_CollectEntities(AR_ExpNode *root, rax *aliases);

/* Traverse an expression tree and add all mentioned attributes:
 * n.attr > 3 to a prefix tree. */
void AR_EXP_CollectAttributes(AR_ExpNode *root, rax *attributes);

/* Search for an aggregation node within the expression tree.
 * Return 1 if one exists.
 * Please note an expression tree can't contain nested aggregation nodes. */
bool AR_EXP_ContainsAggregation(AR_ExpNode *root);

/* Constructs string representation of arithmetic expression tree. */
void AR_EXP_ToString(const AR_ExpNode *root, char **str);

/* Checks to see if expression contains given function.
 * root - expression root to traverse.
 * func - function name to lookup. */
bool AR_EXP_ContainsFunc(const AR_ExpNode *root, const char *func);

/* Returns true if an arithmetic expression node is a constant. */
bool AR_EXP_IsConstant(const AR_ExpNode *exp);

/* Returns true if an arithmetic expression node is a parameter. */
bool AR_EXP_IsParameter(const AR_ExpNode *exp);

/* Returns true if 'exp' represent attribute extraction
 * sets 'attr' to attribute name if provided. */
bool AR_EXP_IsAttribute(const AR_ExpNode *exp, char **attr);

/* Returns true if the arithmetic expression returns
 * a boolean value and false otherwise. */
bool AR_EXP_ReturnsBoolean(const AR_ExpNode *exp);

/* Generate a heap-allocated name for an arithmetic expression.
 * This routine is only used to name ORDER BY expressions. */
char *AR_EXP_BuildResolvedName(AR_ExpNode *root);

/* Clones given expression. */
AR_ExpNode *AR_EXP_Clone(AR_ExpNode *exp);

/* Free arithmetic expression tree. */
void AR_EXP_Free(AR_ExpNode *root);

