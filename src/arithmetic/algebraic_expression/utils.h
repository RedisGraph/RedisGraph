/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../algebraic_expression.h"

#define CHILDREN(node) node->operation.children
#define CHILD_AT(node, idx) CHILDREN(node)[idx]
#define FIRST_CHILD(node) CHILD_AT(node, 0)
#define LAST_CHILD(node) CHILD_AT(node, AlgebraicExpression_ChildCount(node) - 1)

/* Performs inplace re-purposing of an operand into an operation
 * a clone of the original operand is returned. */
AlgebraicExpression *_InplaceRepurposeOperandToOperation
(
	AlgebraicExpression *operand,   // Operand to repurpose.
	AL_EXP_OP op                    // Operation to turn operand into.
);

// Performs inplace re-purposing of an operation into an operand.
void _InplaceRepurposeOperationToOperand
(
    AlgebraicExpression *operation, // Operation to repurpose.
	AlgebraicExpression *operand    // Operand to turn operation into.
);

// Free Operation internals.
void _AlgebraicExpression_FreeOperation
(
	AlgebraicExpression *node   // Operation node to free internals.
);

// Free operand internals.
void _AlgebraicExpression_FreeOperand
(
	AlgebraicExpression *node   // Operand node to free internals.
);

// Locate operand at position `operand_idx` counting from left to right.
AlgebraicExpression *_AlgebraicExpression_GetOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx                    // Operand position (LTR, zero based).
);
