#include "utils.h"
#include "../../util/arr.h"
#include "../../util/rmalloc.h"

/* Performs inplace re-purposing of an operand into an operation
 * a clone of the original operand is returned. */
AlgebraicExpression *_InplaceRepurposeOperandToOperation
(
	AlgebraicExpression *operand,
	AL_EXP_OP op
) {
	AlgebraicExpression *clone = AlgebraicExpression_Clone(operand);
	AlgebraicExpression *operation = AlgebraicExpression_NewOperation(op);
	// turn operand into an operation.
	memcpy(operand, operation, sizeof(AlgebraicExpression));

	// Don't free op internals!
	rm_free(operation);

	return clone;
}

// Performs inplace re-purposing of an operation into an operand.
void _InplaceRepurposeOperationToOperand
(
	AlgebraicExpression *operation,
	AlgebraicExpression *operand
) {
	assert(operation &&
		   operand &&
		   AlgebraicExpression_ChildCount(operation) == 0);

	// Free operation internals.
	_AlgebraicExpression_FreeOperation(operation);
	memcpy(operation, operand, sizeof(AlgebraicExpression));
}

void _AlgebraicExpression_FreeOperation
(
	AlgebraicExpression *node
) {
	assert(node && node->type == AL_OPERATION);

	uint child_count = AlgebraicExpression_ChildCount(node);
	for(uint i = 0; i < child_count; i++) {
		AlgebraicExpression_Free(node->operation.children[i]);
	}
	array_free(node->operation.children);
}

void _AlgebraicExpression_FreeOperand
(
	AlgebraicExpression *node
) {
	assert(node && node->type == AL_OPERAND);
	if(node->operand.free) {
		// TODO: Free matrix.
		// GrB_free(node->operand.matrix);
	}
}

// Locate operand at position `operand_idx` counting from left to right.
AlgebraicExpression *_AlgebraicExpression_GetOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx                    // Operand position (LTR, zero based).
) {
	// `operand_idx` must be within [0, AlgebraicExpression_OperandCount(root)).
	assert(root && operand_idx < AlgebraicExpression_OperandCount(root));

	// Find operand at position `operand_idx`.
	uint current_operand_idx = -1;
	AlgebraicExpression *op = (AlgebraicExpression *)root;
	const AlgebraicExpression *current_operand = root;

	while(op->type == AL_OPERATION) {
		uint child_count = AlgebraicExpression_ChildCount(op);
		for(uint i = 0; i < child_count; i++) {
			// How many operands are there beneath current tree.
			uint sub_tree_child_count = AlgebraicExpression_OperandCount(CHILD_AT(op, i));
			/* Visit subtree if `operand_idx` is less than number of operands in subtree
			 * plus number of operands already skipped. */
			if((sub_tree_child_count + current_operand_idx) > operand_idx) {
				// Investigate subtree.
				op = CHILD_AT(op, i);
				break;
			}
			// Quickly skip subtree.
			current_operand_idx += sub_tree_child_count;
		}
	}

	assert(op->type == AL_OPERAND);
	return op;
}
