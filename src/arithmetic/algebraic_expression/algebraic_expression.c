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

// Remove leftmost child node from 'root' if 'src' is set to true
// rightmost child node otherwise
static AlgebraicExpression *_AlgebraicExpression_RemoveOperand
(
	AlgebraicExpression **root, // Root from which to remove left most child.
	bool src                    // Remove src operand if set, dest otherwise.
) {
	ASSERT(*root);
	bool transpose                   = false;
	AlgebraicExpression *ret         = NULL;
	AlgebraicExpression *parent      = NULL;
	AlgebraicExpression *current     = *root;
	AlgebraicExpression *replacement = NULL;
	AlgebraicExpression **stack      = array_new(AlgebraicExpression *, 0);

	// search for operand
	while(current->type == AL_OPERATION) {
		stack = array_append(stack, current);
		switch(current->operation.op) {
		case AL_EXP_TRANSPOSE:
			transpose = !transpose;
			current = FIRST_CHILD(current); // transpose have only one child
			break;
		case AL_EXP_ADD:
			// Addition order of operands is not effected by transpose
			if(src) current = FIRST_CHILD(current);
			else current = LAST_CHILD(current);
			break;
		case AL_EXP_MUL:
			// Multiplication order of operands depends on transpose
			// | transpose     | src     | get dest |
			// | transpose     | not src | get src  |
			// | not transpose | src     | get src  |
			// | not transpose | not src | get dest |
			if(transpose && src) current = LAST_CHILD(current);
			else if(transpose && !src) current = FIRST_CHILD(current);
			else if(!transpose && src) current = FIRST_CHILD(current);
			else if(!transpose && !src) current = LAST_CHILD(current);

			break;
		default:
			ASSERT("Unknown algebraic expression operation" && false);
		}
	}

	ret = current;
	ASSERT(current->type == AL_OPERAND);

	// expression is just a single operand, set root to NULL
	if(array_len(stack) == 0) *root = NULL;

	/* propegate operand removal upward
	 * when removing A from MUL(A,B) root should become B
	 * when removing A from T(T(A)) root should become NULL
	 * when removing A from ADD(MUL(T(A),B),C) root should become ADD(B,C) */

	while(array_len(stack) > 0) {
		parent = array_pop(stack);
		_AlgebraicExpression_OperationRemoveChild(parent, current);

		// do not free return value
		if(current != ret) AlgebraicExpression_Free(current);

		AL_EXP_OP op = parent->operation.op;
		/* binary operation with a single child, replace operaion with child
		 * removing A from A+B should become B */
		if(op == AL_EXP_ADD || op == AL_EXP_MUL) {
			if(AlgebraicExpression_ChildCount(parent) == 1) {
				// replace operation with only child
				replacement = _AlgebraicExpression_OperationRemoveDest(parent);
				_AlgebraicExpression_InplaceRepurpose(parent, replacement);
			}
			// stop here, no need to propagate further
			break;
		}

		current = parent;
	}

	// handle last parent situation, e.g. removing A from T(T(T(A)))
	if(parent && parent->type == AL_OPERATION) {
		if(AlgebraicExpression_ChildCount(parent) == 0) {
			AlgebraicExpression_Free(parent);
			*root = NULL;
		}
	}

	array_free(stack);
	return ret;
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
	ASSERT(exp);
	switch(exp->type) {
	case AL_OPERATION:
		return _AlgebraicExpression_CloneOperation(exp);
		break;
	case AL_OPERAND:
		return _AlgebraicExpression_CloneOperand(exp);
		break;
	default:
		ASSERT("Unknown algebraic expression node type" && false);
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
		ASSERT("Unknown algebraic expression operation" && false);
		return NULL;
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
	ASSERT(root);
	switch(root->type) {
	case AL_OPERATION:
		return _AlgebraicExpression_Operation_Source(root, transposed);
	case AL_OPERAND:
		return _AlgebraicExpression_Operand_Source(root, transposed);
	default:
		ASSERT("Unknown algebraic expression node type" && false);
		return NULL;
	}
}

// Returns the source entity alias, row domain.
const char *AlgebraicExpression_Source
(
	AlgebraicExpression *root   // Root of expression.
) {
	ASSERT(root);
	return _AlgebraicExpression_Source(root, false);

}

// Returns the destination entity alias represented by the right-most operand column domain.
const char *AlgebraicExpression_Destination
(
	AlgebraicExpression *root   // Root of expression.
) {
	ASSERT(root);
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
	ASSERT(root);

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
		ASSERT("Unknown algebraic expression node type" && false);
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
	if(root == NULL) return false;

	const AlgebraicExpression *n = root;

	// handle directly nested transposes, e.g. T(T(T(X)))
	bool transposed = false;
	while(n->type == AL_OPERATION && n->operation.op == AL_EXP_TRANSPOSE) {
		transposed = !transposed;
		n = FIRST_CHILD(n);
	}

	// TODO: handle cases such as T(A) + T(B).
	return transposed;
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
	ASSERT(op && op->type == AL_OPERAND);
	return op->operand.diagonal;
}

bool _AlgebraicExpression_LocateOperand
(
	AlgebraicExpression *root,      // Root to search
	AlgebraicExpression *proot,     // Root to search
	AlgebraicExpression **operand,  // [output] set to operand
	AlgebraicExpression **parent,   // [output] set to operand parent
	const char *row_domain,         // operand row domain
	const char *column_domain,      // operand column domain
	const char *edge                // operand edge name
) {
	if(root == NULL) return false;

	if(root->type == AL_OPERAND) {
		// check row domain
		if(row_domain != NULL && root->operand.src != NULL) {
			if(strcmp(row_domain, root->operand.src) != 0) {
				return false;
			}
		} else if(row_domain != root->operand.src) {
			return false;
		}

		// check column domain
		if(column_domain != NULL && root->operand.dest != NULL) {
			if(strcmp(column_domain, root->operand.dest) != 0) {
				return false;
			}
		} else if(column_domain != root->operand.dest) {
			return false;
		}

		// check edge
		if(edge != NULL && root->operand.edge != NULL) {
			if(strcmp(edge, root->operand.edge) != 0) {
				return false;
			}
		} else if (edge != root->operand.edge) {
			return false;
		}

		// found seeked operand
		*operand = root;
		if(parent != NULL) *parent = proot;
		return true;
	}

	if(root->type == AL_OPERATION) {
		uint child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			AlgebraicExpression *child = CHILD_AT(root, i);
			if(_AlgebraicExpression_LocateOperand(child, root, operand, parent,
						row_domain, column_domain, edge)) {
				return true;
			}
		}
	}

	return false;
}

bool AlgebraicExpression_LocateOperand
(
	AlgebraicExpression *root,       // Root to search
	AlgebraicExpression **operand,   // [output] set to operand, NULL if missing
	AlgebraicExpression **parent,    // [output] set to operand parent
	const char *row_domain,          // operand row domain
	const char *column_domain,       // operand column domain
	const char *edge                 // operand edge name
) {
	ASSERT(root != NULL);
	ASSERT(operand != NULL);

	*operand = NULL;
	if(parent) *parent = NULL;

	return _AlgebraicExpression_LocateOperand(root, NULL, operand, parent, row_domain,
			column_domain, edge);
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
	ASSERT(root && root->type == AL_OPERATION);
	root->operation.children = array_append(root->operation.children, child);
}

// Remove leftmost child node from root.
AlgebraicExpression *AlgebraicExpression_RemoveSource
(
	AlgebraicExpression **root  // Root from which to remove left most child.
) {
	bool src = true;
	return _AlgebraicExpression_RemoveOperand(root, src);
}

// Remove leftmost child operation node from root.
AlgebraicExpression *AlgebraicExpression_RemoveSourceOp
(
	AlgebraicExpression **root  // Root from which to remove left most child.
) {
	ASSERT(*root);
	ASSERT((*root)->type == AL_OPERATION);

	bool transpose = false;
	AlgebraicExpression *op = NULL;
	AlgebraicExpression *prev = *root;
	AlgebraicExpression *current = *root;

	while(current->type == AL_OPERATION) {
		switch(current->operation.op) {
			case AL_EXP_TRANSPOSE:
				transpose = !transpose;
				op = FIRST_CHILD(current); // transpose have only one child
				break;
			case AL_EXP_ADD:
				// Addition order of operands is not effected by transpose
				op = FIRST_CHILD(current);
				break;
			case AL_EXP_MUL:
				// Multiplication order of operands depends on transpose
				if(transpose) op = LAST_CHILD(current);
				else op = FIRST_CHILD(current);
				break;
			default:
				ASSERT("Unknown algebraic expression operation" && false);
		}
		if(op->type == AL_OPERATION) {
			prev = current;
			current = op;
		}
		else break;
	}

	ASSERT(current->type == AL_OPERATION);

	if(prev == current) {
		/* Expression is just a single operation
		 * return operation and update root to NULL. */
		*root = NULL;
		return current;
	}

	switch(prev->operation.op) {
	case AL_EXP_TRANSPOSE:
	case AL_EXP_ADD:
		/* Transpose ops only have one child and the order of operands for
		 * addition is not modified by transposition, so always remove the source. */
		current = _AlgebraicExpression_OperationRemoveSource(prev);
		break;
	case AL_EXP_MUL:
		/* Remove the destination if we're in a transposed context,
		 * otherwise remove the source. */
		if(transpose) current = _AlgebraicExpression_OperationRemoveDest(prev);
		else current = _AlgebraicExpression_OperationRemoveSource(prev);
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
	bool src = false;
	return _AlgebraicExpression_RemoveOperand(root, src);
}

// Remove rightmost child operation node from root.
AlgebraicExpression *AlgebraicExpression_RemoveDestOp
(
	AlgebraicExpression **root  // Root from which to remove left most child.
) {
	ASSERT(*root);
	ASSERT((*root)->type == AL_OPERATION);

	bool transpose = false;
	AlgebraicExpression *op = NULL;
	AlgebraicExpression *prev = *root;
	AlgebraicExpression *current = *root;

	while(current->type == AL_OPERATION) {
		switch(current->operation.op) {
			case AL_EXP_TRANSPOSE:
				transpose = !transpose;
				op = LAST_CHILD(current); // transpose have only one child
				break;
			case AL_EXP_ADD:
				// Addition order of operands is not effected by transpose
				op = LAST_CHILD(current);
				break;
			case AL_EXP_MUL:
				// Multiplication order of operands depends on transpose
				if(transpose) op = FIRST_CHILD(current);
				else op = LAST_CHILD(current);
				break;
			default:
				ASSERT("Unknown algebraic expression operation" && false);
		}
		if(op->type == AL_OPERATION) {
			prev = current;
			current = op;
		}
		else break;
	}

	ASSERT(current->type == AL_OPERATION);

	if(prev == current) {
		/* Expression is just a single operation
		 * return operation and update root to NULL. */
		*root = NULL;
		return current;
	}

	switch(prev->operation.op) {
	case AL_EXP_TRANSPOSE:
	case AL_EXP_ADD:
		/* Transpose ops only have one child and the order of operands for
		 * addition is not modified by transposition, so always remove the source. */
		current = _AlgebraicExpression_OperationRemoveDest(prev);
		break;
	case AL_EXP_MUL:
		/* Remove the destination if we're in a transposed context,
		 * otherwise remove the source. */
		if(transpose) current = _AlgebraicExpression_OperationRemoveSource(prev);
		else current = _AlgebraicExpression_OperationRemoveDest(prev);
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
	ASSERT(root && m);
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
	ASSERT(root && m);
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
	ASSERT(root && m);
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
	ASSERT(root && m);
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
	ASSERT(root != NULL);
	switch(root->type) {
	case AL_OPERATION:
		_AlgebraicExpression_FreeOperation(root);
		break;
	case AL_OPERAND:
		_AlgebraicExpression_FreeOperand(root);
		break;
	default:
		ASSERT("Unknown algebraic expression node type" && false);
	}
	rm_free(root);
}

