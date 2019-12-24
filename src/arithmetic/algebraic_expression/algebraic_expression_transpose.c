/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../algebraic_expression.h"
#include "utils.h"
#include "../../util/arr.h"

// Transpose addition.
static void _AlgebraicExpression_TransposeAddition
(
	AlgebraicExpression *exp
) {
	// T(A + B) = T(A) + T(B)
	// Transpose children.
	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 0; i < child_count; i++) AlgebraicExpression_Transpose(exp->operation.children[i]);
}

// Transpose multiplication.
static void _AlgebraicExpression_TransposeMultiplication
(
	AlgebraicExpression *exp
) {
	// Swap children, Transpose(A * B) = Transpose(B) * Transpose(A)
	array_reverse(exp->operation.children);
	// Transpose children.
	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 0; i < child_count; i++) AlgebraicExpression_Transpose(exp->operation.children[i]);
}

// Transpose transpose.
static void _AlgebraicExpression_TransposeTranspose
(
	AlgebraicExpression *exp
) {
	// T(T(A)) = A
	// Expecting just a single operand.
	assert(AlgebraicExpression_ChildCount(exp) == 1);
	AlgebraicExpression *only_child = AlgebraicExpression_RemoveRightmostNode(exp);

	// Replace Transpose operation with its child.
	_InplaceRepurposeOperationToOperand(exp, only_child);
}

// Transpose operation.
static void _AlgebraicExpression_TransposeOperation
(
	AlgebraicExpression *exp
) {
	switch(exp->operation.op) {
	case AL_EXP_ADD:
		// T(A + B) = T(A) + T(B)
		_AlgebraicExpression_TransposeAddition(exp);
		break;
	case AL_EXP_MUL:
		_AlgebraicExpression_TransposeMultiplication(exp);
		break;
	case AL_EXP_TRANSPOSE:
		_AlgebraicExpression_TransposeTranspose(exp);
		break;
	default:
		assert("Unknown algebraic expression operation");
		break;
	}
}

// Transpose operand.
static void _AlgebraicExpression_TransposeOperand
(
	AlgebraicExpression *exp
) {
	// No need to transpose a diagonal matrix.
	if(exp->operand.diagonal) return;

	// A -> Transpose(A)
	// We're going to repourpose exp, make a clone.
	AlgebraicExpression *operand = AlgebraicExpression_Clone(exp);
	_InplaceRepurposeOperandToOperation(exp, AL_EXP_TRANSPOSE);

	/* Add original operand as a child of exp (which is now a transpose operation).
	 * Transpose(A) */
	AlgebraicExpression_AddChild(exp, operand);

	// Swap source and destination.
	const char *temp = operand->operand.src;
	operand->operand.src = operand->operand.dest;
	operand->operand.dest = temp;
}

/* Transpose an entire expression recursively.
 * T(T(A)) = A
 * T(A + B) = T(A) + T(B)
 * T(A * B) = T(B) * T(B) */
void AlgebraicExpression_Transpose
(
	AlgebraicExpression *exp    // Expression to transpose.
) {
	assert(exp);

	switch(exp->type) {
	case AL_OPERATION:
		_AlgebraicExpression_TransposeOperation(exp);
		break;

	case AL_OPERAND:
		_AlgebraicExpression_TransposeOperand(exp);
		break;

	default:
		assert("Unknow algebraic expression node type" && false);
	}
}
