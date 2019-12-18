/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#include "../algebraic_expression.h"
#include "utils.h"

//------------------------------------------------------------------------------
// AlgebraicExpression debugging utilities.
//------------------------------------------------------------------------------

static void _AlgebraicExpression_PrintTree
(
	const AlgebraicExpression *exp,
	uint ident
) {
	uint child_count = 0;
	const char *alias = NULL;

	printf("%*s", ident * 4, "");
	switch(exp->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(exp);
		switch(exp->operation.op) {
		case AL_EXP_ADD:
			printf("+\n");
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_PrintTree(exp->operation.children[i], ident + 1);
			}
			break;
		case AL_EXP_MUL:
			printf("*\n");
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_PrintTree(exp->operation.children[i], ident + 1);
			}
			break;
		case AL_EXP_TRANSPOSE:
			printf("Transpose\n");
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_PrintTree(exp->operation.children[i], ident + 1);
			}
			break;
		default:
			assert("Unknown algebraic expression operation");
			break;
		}
		break;
	case AL_OPERAND:
		if(exp->operand.edge) alias = exp->operand.edge;
		else alias = exp->operand.src;
		printf("%s\n", alias);
	default:
		assert("Unknown algebraic expression node type");
		break;
	}
}

void AlgebraicExpression_PrintTree
(
	const AlgebraicExpression *exp  // Root node.
) {
	_AlgebraicExpression_PrintTree(exp, 0);
}

void AlgebraicExpression_Print
(
	const AlgebraicExpression *exp  // Root node.
) {
	assert(exp);
	const char *alias;

	switch(exp->type) {
	case AL_OPERATION:
		switch(exp->operation.op) {
		case AL_EXP_ADD:
			printf("(");
			// Print add first operand.
			AlgebraicExpression_Print(FIRST_CHILD(exp));
			// Expecting at least 2 operands, concat using '+'.
			for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
				printf(" + ");
				AlgebraicExpression_Print(CHILD_AT(exp, i));
			}
			printf(")");
			break;
		case AL_EXP_MUL:
			// Print add first operand.
			AlgebraicExpression_Print(FIRST_CHILD(exp));
			// Expecting at least 2 operands, concat using '*'.
			for(uint i = 1; i < AlgebraicExpression_ChildCount(exp); i++) {
				printf(" * ");
				AlgebraicExpression_Print(CHILD_AT(exp, i));
			}
			break;
		case AL_EXP_TRANSPOSE:
			// Expecting a single child.
			assert(AlgebraicExpression_ChildCount(exp) == 1);
			printf("Transpose(");
			AlgebraicExpression_Print(FIRST_CHILD(exp));
			printf(")");
			break;
		default:
			assert("Unknown algebraic expression operation");
			break;
		}
		break;
	case AL_OPERAND:
		if(exp->operand.edge) alias = exp->operand.edge;
		else alias = exp->operand.src;
		printf("%s", alias);
	default:
		assert("Unknown algebraic expression node type");
		break;
	}
}