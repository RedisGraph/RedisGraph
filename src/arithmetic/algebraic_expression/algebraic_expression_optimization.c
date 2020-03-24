/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#include "../algebraic_expression.h"
#include "utils.h"
#include "../../util/arr.h"

static inline bool _AlgebraicExpression_IsMultiplicationNode(const AlgebraicExpression *node) {
	return (node->type == AL_OPERATION && node->operation.op == AL_EXP_MUL);
}

static inline bool _AlgebraicExpression_IsAdditionNode(const AlgebraicExpression *node) {
	return (node->type == AL_OPERATION && node->operation.op == AL_EXP_ADD);
}

/* Collect all operands under given root by performing a left to right scan
 * adding each encountered operand to the `operands` array. */
static void _AlgebraicExpression_CollectOperands(AlgebraicExpression *root,
												 AlgebraicExpression ***operands) {
	uint child_count = 0;

	switch(root->type) {
	case AL_OPERAND:
		*operands = array_append(*operands, AlgebraicExpression_Clone(root));
		break;
	case AL_OPERATION:
		switch(root->operation.op) {
		case AL_EXP_TRANSPOSE:
			// Transpose is considered as an operand.
			*operands = array_append(*operands, AlgebraicExpression_Clone(root));
			break;
		case AL_EXP_ADD:
		case AL_EXP_MUL:
			child_count = AlgebraicExpression_ChildCount(root);
			for(uint i = 0; i < child_count; i++) {
				_AlgebraicExpression_CollectOperands(CHILD_AT(root, i), operands);
			}
			break;
		default:
			assert("Unknown algebraic expression operation type" && false);
			break;
		}
		break;
	default:
		assert("Unknown algebraic expression node type" && false);
		break;
	}
}

static bool __AlgebraicExpression_MulOverSum(AlgebraicExpression **root) {
	if(_AlgebraicExpression_IsMultiplicationNode(*root)) {
		AlgebraicExpression *l = CHILD_AT((*root), 0);
		AlgebraicExpression *r = CHILD_AT((*root), 1);

		if(_AlgebraicExpression_IsAdditionNode(l) && _AlgebraicExpression_IsAdditionNode(r)) {
			// MATCH ()-[:A|B]->()-[:C|D]->()
			// (A+B)*(C+D) =
			// = (A*C)+(A*D)+(B*C)+(B*D)

			uint left_op_count = AlgebraicExpression_ChildCount(l);
			uint right_op_count = AlgebraicExpression_ChildCount(r);
			AlgebraicExpression **left_ops = array_new(AlgebraicExpression *, left_op_count);
			AlgebraicExpression **right_ops = array_new(AlgebraicExpression *, right_op_count);

			for(uint i = 0; i < left_op_count; i++) {
				left_ops = array_append(left_ops, _AlgebraicExpression_OperationRemoveLeftmostChild(l));
			}
			for(uint i = 0; i < right_op_count; i++) {
				right_ops = array_append(right_ops, _AlgebraicExpression_OperationRemoveLeftmostChild(r));
			}

			assert(AlgebraicExpression_ChildCount(l) == 0 && AlgebraicExpression_ChildCount(r) == 0);

			// Multiply each left op by right op: (A*C), (A*D), (B*C), (B*D).
			AlgebraicExpression **multiplications = array_new(AlgebraicExpression *,
															  left_op_count * right_op_count);
			for(uint i = 0; i < left_op_count; i++) {
				AlgebraicExpression *l_op = left_ops[i];
				for(uint j = 0; j < right_op_count; j++) {
					// Clone op as it's being reused: A*C, A*D.
					if(j > 0) l_op = AlgebraicExpression_Clone(l_op);
					AlgebraicExpression *r_op = right_ops[j];
					// Clone op as it's being reused B*C, B*D.
					if(i > 0) r_op = AlgebraicExpression_Clone(r_op);
					AlgebraicExpression *mul = _AlgebraicExpression_MultiplyToTheRight(l_op, r_op);
					multiplications = array_append(multiplications, mul);
				}
			}
			array_free(left_ops);
			array_free(right_ops);

			// Sum all multiplications: (A*C)+(A*D)+(B*C)+(B*D).
			AlgebraicExpression *add = multiplications[0];
			for(uint i = 1; i < (left_op_count * right_op_count); i++) {
				add = _AlgebraicExpression_AddToTheRight(add, multiplications[i]);
			}

			array_free(multiplications);

			// Free original root and overwrite it with new addition root.
			AlgebraicExpression_Free(*root);
			// Update root.
			*root = add;
			return true;
		}

		else if((_AlgebraicExpression_IsAdditionNode(l) && !_AlgebraicExpression_IsAdditionNode(r)) ||
				(_AlgebraicExpression_IsAdditionNode(r) && !_AlgebraicExpression_IsAdditionNode(l))) {

			// Disconnect left and right children from root.
			r = _AlgebraicExpression_OperationRemoveRightmostChild((*root));
			l = _AlgebraicExpression_OperationRemoveRightmostChild((*root));
			assert(AlgebraicExpression_ChildCount(*root) == 0);

			AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
			AlgebraicExpression *lMul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
			AlgebraicExpression *rMul = AlgebraicExpression_NewOperation(AL_EXP_MUL);

			AlgebraicExpression_AddChild(add, lMul);
			AlgebraicExpression_AddChild(add, rMul);

			AlgebraicExpression *A;
			AlgebraicExpression *B;
			AlgebraicExpression *C;

			if(_AlgebraicExpression_IsAdditionNode(l)) {
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
			}
			// Free original root and overwrite it with new addition root.
			AlgebraicExpression_Free(*root);
			// Update root.
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
				child_count = AlgebraicExpression_OperandCount(root);
				AlgebraicExpression **flat_children = array_new(AlgebraicExpression *, child_count);
				_AlgebraicExpression_CollectOperands(root, &flat_children);
				_AlgebraicExpression_FreeOperation(root);
				root->operation.children = flat_children;
			}

			break;
		default:
			assert("Unknownn algebraic operation type" && false);
			break;
		}
	default:
		break;
	}
}

/* Push down transpose operations to the point where they are applied to individual operands
 * once this optimization is applied there shouldn't be instances of transpose acting on
 * operation nodes such as multiplication and addition.
 *
 * Consider Exp = Transpose(A + B)
 *
 *           (transpose)
 *               (+)
 *         (A)          (B)
 *
 * PushDownTranspose will transform Exp to: Transpose(A) + Transpose(B)
 *
 *                (+)
 *    (transpose)     (transpose)
 *         (A)            (B)
 *
 * Another example, Exp = Transpose(A * B)
 *
 *           (transpose)
 *               (*)
 *         (A)          (B)
 *
 * Would become Transpose(B) * Transpose(A)
 *
 *                (*)
 *    (transpose)     (transpose)
 *         (B)            (A)
 * */
static void _AlgebraicExpression_PushDownTranspose(AlgebraicExpression *root) {
	uint i = 0;
	uint child_count = 0;
	AlgebraicExpression *child = NULL;

	switch(root->type) {
	case AL_OPERAND:
		break;  // Nothing to be done.

	case AL_OPERATION:
		switch(root->operation.op) {
		case AL_EXP_ADD:    // Fall through.
		case AL_EXP_MUL:    // Fall through.
		case AL_EXP_POW:    // Fall through.
			child_count = AlgebraicExpression_ChildCount(root);
			for(; i < child_count; i++) {
				AlgebraicExpression *child = root->operation.children[i];
				_AlgebraicExpression_PushDownTranspose(child);
			}
			break;

		case AL_EXP_TRANSPOSE:
			child = root->operation.children[0];
			if(child->type == AL_OPERATION) {
				/* Transpose operation:
				 * Transpose(A + B) = Transpose(A) + Transpose(B)
				 * Transpose(A * B) = Transpose(B) * Transpose(A) */
				AlgebraicExpression_Transpose(child);
				/* Replace Transpose root with transposed expression.
				 * Remove root only child. */
				_AlgebraicExpression_OperationRemoveRightmostChild(root);
				_AlgebraicExpression_InplaceRepurpose(root, child);

				/* Note, there's no need to dig deep into `root` sub-expression
				 * looking for additional transpose nodes, as calling
				 * AlgebraicExpression_Transpose will remove all of those. */
			}
			break;
		default:
			assert("Unknown operation" && false);
		}
		break;  // Break out of case AL_OPERATION.
	default:
		assert("Unknown algebraic expression node type" && false);
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
	_AlgebraicExpression_PushDownTranspose(*exp);
	_AlgebraicExpression_MulOverSum(exp);
	_AlgebraicExpression_FlattenMultiplications(*exp);
}

