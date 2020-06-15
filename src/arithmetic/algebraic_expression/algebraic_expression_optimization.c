/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/
#include "utils.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../algebraic_expression.h"
#include "../../config.h"

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
		root->operand.bfree = false; // The caller is the new owner of this operand.
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

static bool __AlgebraicExpression_MulOverAdd(AlgebraicExpression **root) {
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
		if(__AlgebraicExpression_MulOverAdd((*root)->operation.children + i)) return true;
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
static void _AlgebraicExpression_MulOverAdd(AlgebraicExpression **root) {
	// As long as the tree changes keep modifying.
	while(__AlgebraicExpression_MulOverAdd(root));
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
			assert("Unknown algebraic operation type" && false);
			break;
		}
	default:
		break;
	}
}

//------------------------------------------------------------------------------
// Transpose pushdown
//------------------------------------------------------------------------------

// Forward declaration.
static void _Pushdown_TransposeExp(AlgebraicExpression *exp);

// Transpose addition.
static void _Pushdown_TransposeAddition
(
	AlgebraicExpression *exp
) {
	// T(A + B) = T(A) + T(B)
	// Transpose children.
	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 0; i < child_count; i++) _Pushdown_TransposeExp(exp->operation.children[i]);
}

// Transpose multiplication.
static void _Pushdown_TransposeMultiplication
(
	AlgebraicExpression *exp
) {
	// Swap children, Transpose(A * B) = Transpose(B) * Transpose(A)
	array_reverse(exp->operation.children);
	// Transpose children.
	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 0; i < child_count; i++) _Pushdown_TransposeExp(exp->operation.children[i]);
}

// Transpose transpose.
static void _Pushdown_TransposeTranspose
(
	AlgebraicExpression *exp
) {
	// T(T(A)) = A
	// Expecting just a single operand.
	assert(AlgebraicExpression_ChildCount(exp) == 1);
	AlgebraicExpression *only_child = _AlgebraicExpression_OperationRemoveRightmostChild(exp);

	// Replace Transpose operation with its child.
	_AlgebraicExpression_InplaceRepurpose(exp, only_child);
}

// Transpose operation.
static void _Pushdown_TransposeOperation
(
	AlgebraicExpression *exp
) {
	switch(exp->operation.op) {
	case AL_EXP_ADD:
		// T(A + B) = T(A) + T(B)
		_Pushdown_TransposeAddition(exp);
		break;
	case AL_EXP_MUL:
		_Pushdown_TransposeMultiplication(exp);
		break;
	case AL_EXP_TRANSPOSE:
		_Pushdown_TransposeTranspose(exp);
		break;
	default:
		assert("Unknown algebraic expression operation");
		break;
	}
}

// Transpose operand.
static void _Pushdown_TransposeOperand
(
	AlgebraicExpression *exp
) {
	// No need to transpose a diagonal matrix.
	if(exp->operand.diagonal) return;

	// A -> Transpose(A)
	// We're going to repurpose exp, make a clone.
	AlgebraicExpression *operand = AlgebraicExpression_Clone(exp);
	_InplaceRepurposeOperandToOperation(exp, AL_EXP_TRANSPOSE);

	/* Add original operand as a child of exp (which is now a transpose operation).
	 * Transpose(A) */
	AlgebraicExpression_AddChild(exp, operand);
}

static void _Pushdown_TransposeExp
(
	AlgebraicExpression *exp
) {
	switch(exp->type) {
	case AL_OPERATION:
		_Pushdown_TransposeOperation(exp);
		break;
	case AL_OPERAND:
		_Pushdown_TransposeOperand(exp);
		break;
	default:
		assert("unknown algebraic expression node type" && false);
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
				_Pushdown_TransposeExp(child);
				/* Replace Transpose root with transposed expression.
				 * Remove root only child. */
				_AlgebraicExpression_OperationRemoveRightmostChild(root);
				_AlgebraicExpression_InplaceRepurpose(root, child);

				/* It is possible for `root` to contain a transpose subexpression
				 * push it further down. */
				_AlgebraicExpression_PushDownTranspose(root);
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
// Replace transpose ops with transposed operands.
//------------------------------------------------------------------------------

// Transpose an operand matrix and update the expression accordingly.
static void _AlgebraicExpression_TransposeOperand(AlgebraicExpression *operand) {
	// Swap the row and column domains of the operand.
	const char *tmp = operand->operand.dest;
	operand->operand.src = operand->operand.dest;
	operand->operand.dest = tmp;

	// Diagonal matrices do not need to be transposed.
	if(operand->operand.diagonal == true) return;

	GrB_Type type;
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix replacement;
	GrB_Matrix A = operand->operand.matrix;
	// Create a new empty matrix with the type and dimensions of the original.
	GrB_Matrix_nrows(&nrows, A);
	GrB_Matrix_ncols(&ncols, A);
	GxB_Matrix_type(&type, A);
	GrB_Info info = GrB_Matrix_new(&replacement, type, nrows, ncols);
	if(info != GrB_SUCCESS) {
		fprintf(stderr, "%s", GrB_error());
		assert(false);
	}

	// Populate the replacement with the transposed contents of the original.
	info = GrB_transpose(replacement, GrB_NULL, GrB_NULL, A, GrB_NULL);
	if(info != GrB_SUCCESS) {
		fprintf(stderr, "%s", GrB_error());
		assert(false);
	}

	// Update the matrix pointer.
	operand->operand.matrix = replacement;
	// As this matrix was constructed, it must ultimately be freed.
	operand->operand.bfree = true;
}

/* Find transpose operations with an operand child and replace them with an actual transposed
 * operand. This optimization should be valid for all transpose operations at this point.
 *
 * If at this point we have the tree:
 *
 *                (+)
 *    (transpose)     (transpose)
 *         (A)            (B)
 *
 * It will become:
 *                (+)
 *           (A')     (B')
 *
 * Similarly, if we have:
 *
 *                (*)
 *    (transpose)     (transpose)
 *         (B)            (A)
 *
 * It will become:
 *                (*)
 *           (B')     (A')
 */
static void _AlgebraicExpression_ApplyTranspose(AlgebraicExpression *root) {
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
			for(uint i = 0; i < child_count; i++) {
				child = root->operation.children[i];
				_AlgebraicExpression_ApplyTranspose(child);
			}
			break;

		case AL_EXP_TRANSPOSE:
			assert(AlgebraicExpression_ChildCount(root) == 1 &&
				   "transpose operation had invalid number of children");
			child = _AlgebraicExpression_OperationRemoveRightmostChild(root);
			// Transpose operands will currently always have an operand child.
			assert(child->type == AL_OPERAND && "encountered unexpected operation as transpose child");
			// Transpose the child operand.
			_AlgebraicExpression_TransposeOperand(child);
			// Replace this operation with the transposed operand.
			_AlgebraicExpression_InplaceRepurpose(root, child);
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
	_AlgebraicExpression_MulOverAdd(exp);
	_AlgebraicExpression_FlattenMultiplications(*exp);

	// Retrieve all operands now that they are guaranteed to be leaves.
	_AlgebraicExpression_PopulateOperands(*exp, QueryCtx_GetGraphCtx());

	// If we are maintaining transposed matrices, all transpose operations have already been replaced.
	if(Config_MaintainTranspose() == false) {
		// Replace transpose operators with actual transposed operands.
		_AlgebraicExpression_ApplyTranspose(*exp);
	}

}

