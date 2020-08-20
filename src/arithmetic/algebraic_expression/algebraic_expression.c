/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "utils.h"
#include "../algebraic_expression.h"
#include "../arithmetic_expression.h"
#include "../../RG.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../util/rmalloc.h"
#include "../../algorithms/algorithms.h"

#include <assert.h>

//------------------------------------------------------------------------------
// Static internal functions.
//------------------------------------------------------------------------------

// Locate the left most node in `exp`
static AlgebraicExpression *_leftMostNode(AlgebraicExpression *exp) {
	AlgebraicExpression *left_most = exp;
	while(left_most->type == AL_OPERATION && AlgebraicExpression_ChildCount(left_most) > 0) {
		left_most = FIRST_CHILD(left_most);
	}
	return left_most;
}

// Locate the right most node in `exp`
static AlgebraicExpression *_rightMostNode(AlgebraicExpression *exp) {
	AlgebraicExpression *right_most = exp;
	while(right_most->type == AL_OPERATION && AlgebraicExpression_ChildCount(right_most) > 0) {
		right_most = LAST_CHILD(right_most);
	}
	return right_most;
}

static AlgebraicExpression *_AlgebraicExpression_CloneOperation
(
	const AlgebraicExpression *exp
) {
	AlgebraicExpression *clone = AlgebraicExpression_NewOperation(exp->operation.op);

	uint child_count = AlgebraicExpression_ChildCount(exp);
	for(uint i = 0; i < child_count; i++) {
		AlgebraicExpression_AddChild(clone, AlgebraicExpression_Clone(exp->operation.children[i]));
	}
	return clone;
}

static AlgebraicExpression *_AlgebraicExpression_CloneOperand
(
	const AlgebraicExpression *exp
) {
	AlgebraicExpression *clone = rm_malloc(sizeof(AlgebraicExpression));
	memcpy(clone, exp, sizeof(AlgebraicExpression));
	return clone;
}

//------------------------------------------------------------------------------
// AlgebraicExpression Node creation functions.
//------------------------------------------------------------------------------

// Create a new AlgebraicExpression operation node.
AlgebraicExpression *AlgebraicExpression_NewOperation
(
	AL_EXP_OP op    // Operation to perform.
) {
	AlgebraicExpression *node = rm_malloc(sizeof(AlgebraicExpression));
	node->type = AL_OPERATION;
	node->operation.op = op;
	node->operation.children = array_new(AlgebraicExpression *, 0);
	return node;
}

// Create a new AlgebraicExpression operand node.
AlgebraicExpression *AlgebraicExpression_NewOperand
(
	GrB_Matrix mat,     // Matrix.
	bool diagonal,      // Is operand a diagonal matrix?
	const char *src,    // Operand row domain (src node).
	const char *dest,   // Operand column domain (destination node).
	const char *edge,   // Operand alias (edge).
	const char *label   // Label attached to matrix.
) {
	AlgebraicExpression *node = rm_malloc(sizeof(AlgebraicExpression));
	node->type = AL_OPERAND;
	node->operand.matrix = mat;
	node->operand.diagonal = diagonal;
	node->operand.bfree = false;
	node->operand.src = src;
	node->operand.dest = dest;
	node->operand.edge = edge;
	node->operand.label = label;
	return node;
}

// Clone algebraic expression node.
AlgebraicExpression *AlgebraicExpression_Clone
(
	const AlgebraicExpression *exp  // Expression to clone.
) {
	assert(exp);
	switch(exp->type) {
	case AL_OPERATION:
		return _AlgebraicExpression_CloneOperation(exp);
		break;
	case AL_OPERAND:
		return _AlgebraicExpression_CloneOperand(exp);
		break;
	default:
		assert("Unknown algebraic expression node type" && false);
		break;
	}
	return NULL;
}

//------------------------------------------------------------------------------
// AlgebraicExpression attributes.
//------------------------------------------------------------------------------

// Forward declaration.
static const char *_AlgebraicExpression_Source(AlgebraicExpression *root, bool transposed);

// Returns the source entity alias (row domain)
// Taking into consideration transpose
static const char *_AlgebraicExpression_Operation_Source
(
	AlgebraicExpression *root,  // Root of expression.
	bool transposed             // Is root transposed
) {
	switch(root->operation.op) {
	case AL_EXP_ADD:
		// Src (A+B) = Src(A)
		// Src (Transpose(A+B)) = Src (Transpose(A)+Transpose(B)) = Src (Transpose(A))
		return _AlgebraicExpression_Source(FIRST_CHILD(root), transposed);
	case AL_EXP_MUL:
		// Src (A*B) = Src(A)
		// Src (Transpose(A*B)) = Src (Transpose(B)*Transpose(A)) = Src (Transpose(B))
		if(transposed) return _AlgebraicExpression_Source(LAST_CHILD(root), transposed);
		else return _AlgebraicExpression_Source(FIRST_CHILD(root), transposed);
	case AL_EXP_TRANSPOSE:
		// Src (Transpose(Transpose(A))) = Src(A)
		// Negate transpose.
		return _AlgebraicExpression_Source(FIRST_CHILD(root), !transposed);
	default:
		assert("Unknown algebraic expression operation" && false);
	}
}

// Returns the source entity alias (row domain)
// Taking into consideration transpose
static const char *_AlgebraicExpression_Operand_Source
(
	AlgebraicExpression *root,  // Root of expression.
	bool transposed             // Is root transposed
) {
	return (transposed) ? root->operand.dest : root->operand.src;
}

// Returns the source entity alias (row domain)
// Taking into consideration transpose
static const char *_AlgebraicExpression_Source
(
	AlgebraicExpression *root,  // Root of expression.
	bool transposed             // Is root transposed
) {
	assert(root);
	switch(root->type) {
	case AL_OPERATION:
		return _AlgebraicExpression_Operation_Source(root, transposed);
	case AL_OPERAND:
		return _AlgebraicExpression_Operand_Source(root, transposed);
	default:
		assert("Unknown algebraic expression node type" && false);
	}
}

// Returns the source entity alias, row domain.
const char *AlgebraicExpression_Source
(
	AlgebraicExpression *root   // Root of expression.
) {
	assert(root);
	return _AlgebraicExpression_Source(root, false);

}

// Returns the destination entity alias represented by the right-most operand column domain.
const char *AlgebraicExpression_Destination
(
	AlgebraicExpression *root   // Root of expression.
) {
	assert(root);
	// Dest(exp) = Src(Transpose(exp))
	// Gotta love it!
	return _AlgebraicExpression_Source(root, true);
}

/* Returns the first edge alias encountered.
 * if no edge alias is found NULL is returned. */
const char *AlgebraicExpression_Edge
(
	const AlgebraicExpression *root   // Root of expression.
) {
	assert(root);

	uint child_count = 0;
	const char *edge = NULL;
	switch(root->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			edge = AlgebraicExpression_Edge(CHILD_AT(root, i));
			if(edge) return edge;
		}
		break;
	case AL_OPERAND:
		return root->operand.edge;
	}

	return NULL;
}

// Returns the number of child nodes directly under root.
uint AlgebraicExpression_ChildCount
(
	const AlgebraicExpression *root   // Root of expression.
) {
	// Empty expression.
	if(!root) return 0;

	if(root->type == AL_OPERATION) return array_len(root->operation.children);
	else return 0;
}

// Returns the number of operands in expression.
uint AlgebraicExpression_OperandCount
(
	const AlgebraicExpression *root
) {
	// Empty expression.
	if(!root) return 0;

	uint operand_count = 0;
	uint child_count = 0;
	switch(root->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			operand_count += AlgebraicExpression_OperandCount(CHILD_AT(root, i));
		}
		break;
	case AL_OPERAND:
		operand_count = 1;
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
		break;
	}
	return operand_count;
}

// Returns the number of operations in expression.
uint AlgebraicExpression_OperationCount
(
	const AlgebraicExpression *root,
	AL_EXP_OP op_type
) {
	// Empty expression.
	if(!root) return 0;

	uint op_count = 0;
	if(root->type == AL_OPERATION) {
		if(root->operation.op & op_type) op_count = 1;
		uint child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			op_count += AlgebraicExpression_OperationCount(CHILD_AT(root, i), op_type);
		}
	}
	return op_count;
}

// Returns true if entire expression is transposed.
bool AlgebraicExpression_Transposed
(
	const AlgebraicExpression *root   // Root of expression.
) {
	// Empty expression.
	if(!root) return false;

	//TODO: handle cases such as T(A) + T(B).
	return (root->type == AL_OPERATION && root->operation.op == AL_EXP_TRANSPOSE);
}

// Returns true if expression contains operation.
bool AlgebraicExpression_ContainsOp
(
	const AlgebraicExpression *root,
	AL_EXP_OP op
) {
	// Empty expression.
	if(!root) return false;

	if(root->type == AL_OPERATION) {
		if(root->operation.op == op) return true;
		uint child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			if(AlgebraicExpression_ContainsOp(CHILD_AT(root, i), op)) return true;
		}
	}
	return false;
}

// Checks to see if operand at position `operand_idx` is a diagonal matrix.
bool AlgebraicExpression_DiagonalOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx                    // Operand position (LTR, zero based).
) {
	// Empty expression.
	if(!root) return false;

	const AlgebraicExpression *op = _AlgebraicExpression_GetOperand(root, operand_idx);
	assert(op && op->type == AL_OPERAND);
	return op->operand.diagonal;
}

//------------------------------------------------------------------------------
// AlgebraicExpression modification functions.
//------------------------------------------------------------------------------

// Adds child node to root children list.
void AlgebraicExpression_AddChild
(
	AlgebraicExpression *root,  // Root to attach child to.
	AlgebraicExpression *child  // Child node to attach.
) {
	assert(root && root->type == AL_OPERATION);
	root->operation.children = array_append(root->operation.children, child);
}

// Remove leftmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveSource
(
	AlgebraicExpression **root  // Root from which to remove left most child.
) {
	assert(*root);
	bool transpose = false;
	AlgebraicExpression *prev = *root;
	AlgebraicExpression *current = *root;

	while(current->type == AL_OPERATION) {
		if(current->operation.op == AL_EXP_TRANSPOSE) transpose = !transpose;
		prev = current;
		if(transpose) current = LAST_CHILD(current);
		else current = FIRST_CHILD(current);
	}
	ASSERT(current->type == AL_OPERAND);

	if(prev == current) {
		/* Expression is just a single operand
		 * return operand and update root to NULL. */
		*root = NULL;
		return current;
	}

	switch(prev->operation.op) {
	case AL_EXP_TRANSPOSE:
	case AL_EXP_ADD:
		/* Transpose ops only have one child and the order of operands for
		 * addition is not modified by transposition, so always remove the source. */
		_AlgebraicExpression_OperationRemoveSource(prev);
		break;
	case AL_EXP_MUL:
		// Remove the destination if we're in a transposed context,
		// otherwise remove the source.
		if(transpose) _AlgebraicExpression_OperationRemoveDest(prev);
		else _AlgebraicExpression_OperationRemoveSource(prev);
		break;
	default:
		ASSERT("Unknown algebraic expression operation" && false);
	}

	uint child_count = AlgebraicExpression_ChildCount(prev);
	if(child_count == 1) {
		/* If we just previous operation only has one remaining child,
		 * it can be replaced by that child:
		 * MUL(A,B) after removing A will become just B
		 * Currently, this point is unreachable for transpose ops,
		 * as at this point their child is always an operation.
		 * If that changes, logic should be added such that:
		 * TRANSPOSE(A) after removing A should become NULL. */
		AlgebraicExpression *replacement = _AlgebraicExpression_OperationRemoveDest(prev);
		_AlgebraicExpression_InplaceRepurpose(prev, replacement);
	}
	return current;
}

// Remove right most child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveDest
(
	AlgebraicExpression **root  // Root from which to remove left most child.
) {
	assert(*root);
	bool transpose = false;
	AlgebraicExpression *prev = *root;
	AlgebraicExpression *current = *root;

	while(current->type == AL_OPERATION) {
		if(current->operation.op == AL_EXP_TRANSPOSE) transpose = !transpose;
		prev = current;
		if(transpose) current = FIRST_CHILD(current);
		else current = LAST_CHILD(current);
	}
	ASSERT(current->type == AL_OPERAND);

	if(prev == current) {
		/* Expression is just a single operand
		 * return operand and update root to NULL. */
		*root = NULL;
		return current;
	}

	switch(prev->operation.op) {
	case AL_EXP_TRANSPOSE:
	case AL_EXP_ADD:
		/* Transpose ops only have one child and the order of operands for
		 * addition is not modified by transposition, so always remove the destination. */
		_AlgebraicExpression_OperationRemoveDest(prev);
		break;
	case AL_EXP_MUL:
		// Remove the source if we're in a transposed context,
		// otherwise remove the destination.
		if(transpose) _AlgebraicExpression_OperationRemoveSource(prev);
		else _AlgebraicExpression_OperationRemoveDest(prev);
		break;
	default:
		ASSERT("Unknown algebraic expression operation" && false);
	}

	uint child_count = AlgebraicExpression_ChildCount(prev);
	if(child_count == 1) {
		/* If we just previous operation only has one remaining child,
		 * it can be replaced by that child:
		 * MUL(A,B) after removing A will become just B
		 * Currently, this point is unreachable for transpose ops,
		 * as at this point their child is always an operation.
		 * If that changes, logic should be added such that:
		 * TRANSPOSE(A) after removing A should become NULL. */
		AlgebraicExpression *replacement = _AlgebraicExpression_OperationRemoveDest(prev);
		_AlgebraicExpression_InplaceRepurpose(prev, replacement);
	}
	return current;
}

/* Multiply root to the left with op.
 * Updates root. */
void AlgebraicExpression_MultiplyToTheLeft
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *rhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current left most operand. */
	AlgebraicExpression *left_most_operand = _leftMostNode(rhs);
	AlgebraicExpression *lhs = AlgebraicExpression_NewOperand(m, false, left_most_operand->operand.src,
															  left_most_operand->operand.dest, NULL, NULL);

	*root = _AlgebraicExpression_MultiplyToTheLeft(lhs, rhs);
}

/* Multiply root to the right with op.
 * Updates root. */
void AlgebraicExpression_MultiplyToTheRight
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *lhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current right most operand. */
	AlgebraicExpression *right_most_operand = _rightMostNode(lhs);
	AlgebraicExpression *rhs = AlgebraicExpression_NewOperand(m, false, right_most_operand->operand.src,
															  right_most_operand->operand.dest, NULL, NULL);

	*root = _AlgebraicExpression_MultiplyToTheRight(lhs, rhs);
}

// Add expression to the left by operand
// A + (exp)
void AlgebraicExpression_AddToTheLeft
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *rhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current left most operand. */
	AlgebraicExpression *left_most_operand = _leftMostNode(rhs);
	AlgebraicExpression *lhs = AlgebraicExpression_NewOperand(m, false, left_most_operand->operand.src,
															  left_most_operand->operand.dest, left_most_operand->operand.edge, NULL);

	*root = _AlgebraicExpression_AddToTheLeft(lhs, rhs);
}

// Add expression to the right by operand
// (exp) + A
void AlgebraicExpression_AddToTheRight
(
	AlgebraicExpression **root,
	GrB_Matrix m
) {
	assert(root && m);
	AlgebraicExpression *lhs = *root;
	/* Assuming new operand inherits (src, dest and edge) from
	 * from the current right most operand. */
	AlgebraicExpression *right_most_operand = _rightMostNode(lhs);
	AlgebraicExpression *rhs = AlgebraicExpression_NewOperand(m, false, right_most_operand->operand.src,
															  right_most_operand->operand.dest, right_most_operand->operand.edge, NULL);

	*root = _AlgebraicExpression_AddToTheRight(lhs, rhs);
}

//------------------------------------------------------------------------------
// AlgebraicExpression free
//------------------------------------------------------------------------------

// Free algebraic expression.
void AlgebraicExpression_Free
(
	AlgebraicExpression *root  // Root node.
) {
	assert(root);
	switch(root->type) {
	case AL_OPERATION:
		_AlgebraicExpression_FreeOperation(root);
		break;
	case AL_OPERAND:
		_AlgebraicExpression_FreeOperand(root);
		break;
	default:
		assert("Unknow algebraic expression node type" && false);
	}
	rm_free(root);
}

