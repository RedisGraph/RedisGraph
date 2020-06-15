#include "utils.h"
#include "../../config.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

/* Performs inplace re-purposing of an operand into an operation. */
void _InplaceRepurposeOperandToOperation
(
	AlgebraicExpression *operand,
	AL_EXP_OP op
) {
	assert(operand && operand->type == AL_OPERAND);
	AlgebraicExpression *operation = AlgebraicExpression_NewOperation(op);
	// turn operand into an operation.
	memcpy(operand, operation, sizeof(AlgebraicExpression));

	// Don't free op internals!
	rm_free(operation);
}

// Performs inplace re-purposing of an operation into an operand.
void _AlgebraicExpression_InplaceRepurpose
(
	AlgebraicExpression *exp,
	AlgebraicExpression *replacement
) {
	assert(exp && replacement && AlgebraicExpression_ChildCount(exp) == 0);
	// Free internals.
	if(exp->type == AL_OPERATION) _AlgebraicExpression_FreeOperation(exp);
	else if(exp->type == AL_OPERAND) _AlgebraicExpression_FreeOperand(exp);
	else assert("Unknown algebraic expression type" && false);

	// Replace.
	memcpy(exp, replacement, sizeof(AlgebraicExpression));
	// Free the memory of the migrated replacement.
	rm_free(replacement);
}

// Removes the rightmost direct child node of root.
AlgebraicExpression *_AlgebraicExpression_OperationRemoveRightmostChild
(
	AlgebraicExpression *root  // Root from which to remove a child.
) {
	assert(root);
	if(root->type != AL_OPERATION) return NULL;

	// No child nodes to remove.
	if(AlgebraicExpression_ChildCount(root) == 0) return NULL;

	// Remove rightmost child.
	AlgebraicExpression *child = array_pop(root->operation.children);
	return child;
}

// Removes the leftmost direct child node of root.
AlgebraicExpression *_AlgebraicExpression_OperationRemoveLeftmostChild
(
	AlgebraicExpression *root   // Root from which to remove a child.
) {
	assert(root);
	if(root->type != AL_OPERATION) return NULL;

	uint child_count = AlgebraicExpression_ChildCount(root);
	// No child nodes to remove.
	if(child_count == 0) return NULL;

	// Remove leftmost child.
	AlgebraicExpression *child = root->operation.children[0];

	// Shift left by 1.
	for(uint i = 0; i < child_count - 1; i++) {
		root->operation.children[i] = root->operation.children[i + 1];
	}
	array_pop(root->operation.children);

	return child;
}

/* Multiplies `exp` to the left by `lhs`.
 * Returns new expression root.
 * `lhs` = (A + B)
 * `exp` = Transpose(C)
 * Returns (A + B) * Transpose(C) where `*` is the new root. */
AlgebraicExpression *_AlgebraicExpression_MultiplyToTheLeft
(
	AlgebraicExpression *lhs,
	AlgebraicExpression *exp
) {
	assert(lhs && exp);
	AlgebraicExpression *mul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
	AlgebraicExpression_AddChild(mul, lhs);
	AlgebraicExpression_AddChild(mul, exp);
	return mul;
}

/* Multiplies `exp` to the right by `rhs`.
 * Returns new expression root.
 * `exp` = Transpose(C)
 * `rhs` = (A + B)
 * Returns Transpose(C) * (A + B) where `*` is the new root. */
AlgebraicExpression *_AlgebraicExpression_MultiplyToTheRight
(
	AlgebraicExpression *exp,
	AlgebraicExpression *rhs
) {
	assert(exp && rhs);
	AlgebraicExpression *mul = AlgebraicExpression_NewOperation(AL_EXP_MUL);
	AlgebraicExpression_AddChild(mul, exp);
	AlgebraicExpression_AddChild(mul, rhs);
	return mul;
}

/* Adds `exp` to the left by `lhs`.
 * Returns new expression root.
 * `lhs` = (A * B)
 * `exp` = Transpose(C)
 * Returns (A * B) + Transpose(C) where `+` is the new root. */
AlgebraicExpression *_AlgebraicExpression_AddToTheLeft
(
	AlgebraicExpression *lhs,
	AlgebraicExpression *exp
) {
	assert(lhs && exp);
	AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
	AlgebraicExpression_AddChild(add, lhs);
	AlgebraicExpression_AddChild(add, exp);
	return add;
}

/* Adds `exp` to the right by `rhs`.
 * Returns new expression root.
 * `exp` = Transpose(C)
 * `rhs` = (A * B)
 * Returns Transpose(C) + (A * B) where `+` is the new root. */
AlgebraicExpression *_AlgebraicExpression_AddToTheRight
(
	AlgebraicExpression *exp,
	AlgebraicExpression *rhs
) {
	assert(exp && rhs);
	AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
	AlgebraicExpression_AddChild(add, exp);
	AlgebraicExpression_AddChild(add, rhs);
	return add;
}

void _AlgebraicExpression_FreeOperation
(
	AlgebraicExpression *node
) {
	assert(node && node->type == AL_OPERATION);
	if(node->operation.children) {
		uint child_count = AlgebraicExpression_ChildCount(node);
		for(uint i = 0; i < child_count; i++) {
			AlgebraicExpression_Free(node->operation.children[i]);
		}
		array_free(node->operation.children);
		node->operation.children = NULL;
	}
}

void _AlgebraicExpression_FreeOperand
(
	AlgebraicExpression *node
) {
	assert(node && node->type == AL_OPERAND);
	if(node->operand.bfree) GrB_Matrix_free(&node->operand.matrix);
}

// Locate operand at position `operand_idx` counting from left to right.
AlgebraicExpression *__AlgebraicExpression_GetOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx,                   // Operand position (LTR, zero based).
	uint *current_operand_idx
) {
	// `operand_idx` must be within [0, AlgebraicExpression_OperandCount(root)).
	assert(root);

	uint child_count = 0;
	AlgebraicExpression *operand = NULL;

	switch(root->type) {
	case  AL_OPERAND:
		if(operand_idx == *current_operand_idx) return (AlgebraicExpression *)root;
		*current_operand_idx += 1;
		break;
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		for(uint i = 0; i < child_count; i++) {
			operand = __AlgebraicExpression_GetOperand(CHILD_AT(root, i), operand_idx, current_operand_idx);
			if(operand) return operand;
		}
		break;
	default:
		assert("unknown algebraic expression node type" && false);
	}
	return NULL;
}

AlgebraicExpression *_AlgebraicExpression_GetOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx                    // Operand position (LTR, zero based).
) {
	uint current_operand_idx = 0;
	return __AlgebraicExpression_GetOperand(root, operand_idx, &current_operand_idx);
}

// Populate an operand with a standard matrix.
static void _AlgebraicExpression_PopulateOperand(AlgebraicExpression *operand,
												 const GraphContext *gc) {
	/* Do not update matrix if already set, as algebraic expression test depends on this behavior.
	 * TODO Redesign _AlgebraicExpression_FromString to remove this condition. */
	if(operand->operand.matrix != GrB_NULL) return;

	GrB_Matrix m = GrB_NULL;
	const char *label = operand->operand.label;
	if(label == NULL) {
		m = Graph_GetAdjacencyMatrix(gc->g);
	} else if(operand->operand.diagonal) {
		Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_NODE);
		if(!s) m = Graph_GetZeroMatrix(gc->g);
		else m = Graph_GetLabelMatrix(gc->g, s->id);
	} else {
		Schema *s = GraphContext_GetSchema(gc, label, SCHEMA_EDGE);
		if(!s) m = Graph_GetZeroMatrix(gc->g);
		else m = Graph_GetRelationMatrix(gc->g, s->id);
	}
	operand->operand.matrix = m;
}

// Populate a transposed operand with a transposed relationship matrix and swap the row/col domains.
static void _AlgebraicExpression_PopulateTransposedOperand(AlgebraicExpression *operand,
														   const GraphContext *gc) {
	// Swap the row and column domains of the operand.
	const char *tmp = operand->operand.dest;
	operand->operand.src = operand->operand.dest;
	operand->operand.dest = tmp;

	// Diagonal matrices do not need to be transposed.
	if(operand->operand.diagonal == true) return;

	/* Do not update matrix if already set, as algebraic expression test depends on this behavior.
	 * TODO Redesign _AlgebraicExpression_FromString to remove this condition. */
	if(operand->operand.matrix != GrB_NULL) return;

	GrB_Matrix m = GrB_NULL;
	const char *label = operand->operand.label;
	if(label == NULL) {
		m = Graph_GetTransposedAdjacencyMatrix(gc->g);
	} else {
		Schema *s = GraphContext_GetSchema(gc, operand->operand.label, SCHEMA_EDGE);
		if(!s) m = Graph_GetZeroMatrix(gc->g);
		else m = Graph_GetTransposedRelationMatrix(gc->g, s->id);
	}
	operand->operand.matrix = m;
}

// TODO this function is only used within AlgebraicExpression_Optimize, consider moving it.
// Fetch all operands, replacing transpose operations with transposed operands if they are available.
void _AlgebraicExpression_PopulateOperands(AlgebraicExpression *root, const GraphContext *gc) {
	uint child_count = 0;
	switch(root->type) {
	case AL_OPERATION:
		child_count = AlgebraicExpression_ChildCount(root);
		// If we are maintaining transposed matrices, it can be retrieved now.
		if(root->operation.op == AL_EXP_TRANSPOSE && Config_MaintainTranspose()) {
			assert(child_count == 1 && "Transpose operation had invalid number of children");
			AlgebraicExpression *child = _AlgebraicExpression_OperationRemoveRightmostChild(root);
			// Fetch the transposed matrix and update the operand.
			_AlgebraicExpression_PopulateTransposedOperand(child, gc);
			// Replace this operation with the transposed operand.
			_AlgebraicExpression_InplaceRepurpose(root, child);
			break;
		}
		for(uint i = 0; i < child_count; i++) {
			_AlgebraicExpression_PopulateOperands(CHILD_AT(root, i), gc);
		}
		break;
	case AL_OPERAND:
		_AlgebraicExpression_PopulateOperand(root, gc);
		break;
	default:
		assert("Unknown algebraic expression node type" && false);
		break;
	}
}

