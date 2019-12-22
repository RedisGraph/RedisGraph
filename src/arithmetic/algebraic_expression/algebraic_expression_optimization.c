/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#include "../algebraic_expression.h"
#include "utils.h"
#include "../../util/arr.h"

static bool _AlgebraicExpression_IsMultiplicationNode(const AlgebraicExpression *node) {
	return (node->type == AL_OPERATION && node->operation.op == AL_EXP_MUL);
}

static bool _AlgebraicExpression_IsAdditionNode(const AlgebraicExpression *node) {
	return (node->type == AL_OPERATION && node->operation.op == AL_EXP_ADD);
}

/* Collect all operands under given root by performing a left to right scan
 * adding each encountered operand to the `operands` array. */
static void _AlgebraicExpression_CollectOperands(AlgebraicExpression *root,
												 AlgebraicExpression ***operands) {
	uint child_count = 0;

	switch(root->type) {
	case AL_OPERAND:
		*operands = array_append(*operands, root);
		break;
	case AL_OPERATION:
		switch(root->operation.op) {
		case AL_EXP_TRANSPOSE:
			// Transpose is considered as an operand.
			*operands = array_append(*operands, root);
			break;
		case AL_EXP_ADD:
		case AL_EXP_MUL:
			child_count = AlgebraicExpression_ChildCount(root);
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_CollectOperands(AlgebraicExpression_Clone(CHILD_AT(root, i)), operands);
			}
			break;
		default:
			assert("Unknow algebraic expression operation type" && false);
			break;
		}
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
		break;
	}
}

static bool __AlgebraicExpression_MulOverSum(AlgebraicExpression **root) {
	if(_AlgebraicExpression_IsMultiplicationNode(*root)) {
		AlgebraicExpression *l = (*root)->operation.children[0];
		AlgebraicExpression *r = (*root)->operation.children[1];

		// Do not care for (A + B) * (C + D)
		// As this will end up performing A*C + A*D + B*C + B*D
		// which is 4 multiplications and 3 additions compared to the original
		// 2 additions and one multiplication.
		if((_AlgebraicExpression_IsAdditionNode(l) && !_AlgebraicExpression_IsAdditionNode(r)) ||
		   (_AlgebraicExpression_IsAdditionNode(r) && !_AlgebraicExpression_IsAdditionNode(l))) {

			AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
			AlgebraicExpression *lMul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
			AlgebraicExpression *rMul = AlgebraicExpression_NewOperation(AL_EXP_MUL);

			AlgebraicExpression_AddChild(add, lMul);
			AlgebraicExpression_AddChild(add, rMul);

			AlgebraicExpression *A;
			AlgebraicExpression *B;
			AlgebraicExpression *C;

			if(l->operation.op == AL_EXP_ADD) {
				// Lefthand side is addition.
				// (A + B) * C = (A * C) + (B * C)

				A = _AlgebraicExpression_OperationRemoveLeftmostChild(l);
				B = _AlgebraicExpression_OperationRemoveRightmostChild(l);
				C = r;

				AlgebraicExpression_Free(l);
				AlgebraicExpression_AddChild(lMul, A);
				AlgebraicExpression_AddChild(lMul, C);
				AlgebraicExpression_AddChild(rMul, B);
				AlgebraicExpression_AddChild(rMul, AlgebraicExpression_Clone(C));

				// TODO: Mark r as reusable.
				// if(r->type == AL_OPERATION) r->operation.reusable = true;
			} else {
				// Righthand side is addition.
				// C * (A + B) = (C * A) + (C * B)

				A = _AlgebraicExpression_OperationRemoveLeftmostChild(r);
				B = _AlgebraicExpression_OperationRemoveRightmostChild(r);
				C = l;

				AlgebraicExpression_Free(r);
				AlgebraicExpression_AddChild(lMul, C);
				AlgebraicExpression_AddChild(lMul, A);
				AlgebraicExpression_AddChild(rMul, AlgebraicExpression_Clone(C));
				AlgebraicExpression_AddChild(rMul, B);

				// TODO: Mark r as reusable.
				// if(l->type == AL_OPERATION) l->operation.reusable = true;
			}
			*root = add;
			return true;
		}
	}

	// Recurse.
	uint child_count = AlgebraicExpression_ChildCount(*root);
	for(uint i = 0; i < child_count; i++) {
		if(__AlgebraicExpression_MulOverSum((*root)->operation.children + i)) return true;
	}
	return false;
}

// Distributive, multiplication over addition:
// A * (B + C) = (A * B) + (A * C)
//
//           (*)
//   (A)             (+)
//            (B)          (C)
//
// Becomes
//
//               (+)
//       (*)                (*)
// (A)        (B)     (A)        (C)
//
// Whenever we encounter a multiplication operation
// where one child is an addition operation and the other child
// is a multiplication operation, we'll replace root multiplication
// operation with an addition operation with two multiplication operations
// one for each child of the original addition operation, as can be seen above.
// we'll want to reuse the left handside of the multiplication.
static void _AlgebraicExpression_MulOverSum(AlgebraicExpression **root) {
	// As long as the tree changes keep modifying.
	while(__AlgebraicExpression_MulOverSum(root));
}

/* Collapse multiplication operation under a single multiplication op
 * exp = A * B * C
 * exp can be computed in two ways:
 * 1. (A * B) * C
 * 2. A * (B * C)
 * by flattening the expression both 1 and 2 are represented by the
 * same tree structure. */
static void _AlgebraicExpression_FlattenMultiplications(AlgebraicExpression *root) {
	assert(root);
	uint child_count;

	switch(root->type) {
	case AL_OPERATION:
		switch(root->operation.op) {
		case AL_EXP_ADD:
		case AL_EXP_TRANSPOSE:
			// Keep searching for a multiplication operation.
			child_count = AlgebraicExpression_ChildCount(root);
			for(int i = 0; i < child_count; i++) {
				_AlgebraicExpression_FlattenMultiplications(CHILD_AT(root, i));
			}
			break;

		case AL_EXP_MUL:
			// Root has sub multiplication node(s).
			if(AlgebraicExpression_OperationCount(root, AL_EXP_MUL) > 1) {
				child_count = AlgebraicExpression_ChildCount(root);
				AlgebraicExpression **flat_children = array_new(AlgebraicExpression *, child_count);
				_AlgebraicExpression_CollectOperands(root, &flat_children);
				// Free `old` child array.
				for(uint i = 0; i < child_count; i++) AlgebraicExpression_Free(CHILD_AT(root, i));
				array_free(root->operation.children);
				root->operation.children = flat_children;
			}

			break;
		default:
			assert("Unknown algebraic operation type" && false);
			break;
		}
	default:
		break;
	}
}

//------------------------------------------------------------------------------
// AlgebraicExpression optimizations
//------------------------------------------------------------------------------
void AlgebraicExpression_Optimize
(
	AlgebraicExpression **exp
) {
	assert(exp);
	_AlgebraicExpression_MulOverSum(exp);
	_AlgebraicExpression_FlattenMultiplications(*exp);
}
