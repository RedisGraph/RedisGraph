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
	ASSERT(operand && operand->type == AL_OPERAND);
	AlgebraicExpression *operation = AlgebraicExpression_NewOperation(op);
	// turn operand into an operation.

	AlgExpReference_Free(operand->operand.reference);
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
	ASSERT(exp && replacement && AlgebraicExpression_ChildCount(exp) == 0);
	// Free internals.
	if(exp->type == AL_OPERATION) _AlgebraicExpression_FreeOperation(exp);
	else if(exp->type == AL_OPERAND) _AlgebraicExpression_FreeOperand(exp);
	else ASSERT("Unknown algebraic expression type" && false);

	// Replace.
	memcpy(exp, replacement, sizeof(AlgebraicExpression));
	// Free the memory of the migrated replacement.
	rm_free(replacement);
}

void _AlgebraicExpression_OperationRemoveChild
(
	AlgebraicExpression *parent,
	const AlgebraicExpression *child
) {
	ASSERT(parent != NULL);
	ASSERT(child != NULL);

	if(parent->type != AL_OPERATION) return;

	uint child_count = AlgebraicExpression_ChildCount(parent);
	// no child nodes to remove
	if(child_count == 0) return;

	// search for child in parent
	for(uint i = 0; i < child_count; i++) {
		if(parent->operation.children[i] != child) continue;

		// child found, remove it
		// shift-left following children
		for(uint j = i; j < child_count - 1; j++) {
			parent->operation.children[j] = parent->operation.children[j+1];
		}
		array_pop(parent->operation.children);
		break;
	}
}

// Removes the rightmost direct child node of root.
AlgebraicExpression *_AlgebraicExpression_OperationRemoveDest
(
	AlgebraicExpression *root  // Root from which to remove a child.
) {
	ASSERT(root);
	if(root->type != AL_OPERATION) return NULL;

	// No child nodes to remove.
	if(AlgebraicExpression_ChildCount(root) == 0) return NULL;

	// Remove rightmost child.
	AlgebraicExpression *child = array_pop(root->operation.children);
	return child;
}

// Removes the leftmost direct child node of root.
AlgebraicExpression *_AlgebraicExpression_OperationRemoveSource
(
	AlgebraicExpression *root   // Root from which to remove a child.
) {
	ASSERT(root);
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
	ASSERT(lhs && exp);
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
	ASSERT(exp && rhs);
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
	ASSERT(lhs && exp);
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
	ASSERT(exp && rhs);
	AlgebraicExpression *add = AlgebraicExpression_NewOperation(AL_EXP_ADD);
	AlgebraicExpression_AddChild(add, exp);
	AlgebraicExpression_AddChild(add, rhs);
	return add;
}

void _AlgebraicExpression_FreeOperation
(
	AlgebraicExpression *node
) {
	ASSERT(node && node->type == AL_OPERATION);
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
	ASSERT(node && node->type == AL_OPERAND);
	if(node->operand.bfree) GrB_Matrix_free(&node->operand.matrix);
	AlgExpReference_Free(node->operand.reference);
}

// Locate operand at position `operand_idx` counting from left to right.
AlgebraicExpression *__AlgebraicExpression_GetOperand
(
	const AlgebraicExpression *root,    // Root of expression.
	uint operand_idx,                   // Operand position (LTR, zero based).
	uint *current_operand_idx
) {
	// `operand_idx` must be within [0, AlgebraicExpression_OperandCount(root)).
	ASSERT(root);

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
		ASSERT("unknown algebraic expression node type" && false);
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
	operand->operand.dest = operand->operand.src;
	operand->operand.src = tmp;

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
		bool maintain_transpose = false;
		Config_Option_get(Config_MAINTAIN_TRANSPOSE, &maintain_transpose);
		if(root->operation.op == AL_EXP_TRANSPOSE && maintain_transpose) {
			ASSERT(child_count == 1 && "Transpose operation had invalid number of children");
			AlgebraicExpression *child = _AlgebraicExpression_OperationRemoveDest(root);
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
		ASSERT("Unknown algebraic expression node type" && false);
		break;
	}
}

void AlgebraicExpression_PopulateReferences(AlgebraicExpression *exp, PathPatternCtx *pathPatternCtx) {
	switch(exp->type) {
		case AL_OPERATION: {
			uint child_count = AlgebraicExpression_ChildCount(exp);
			for (uint i = 0; i < child_count; i++) {
				AlgebraicExpression_PopulateReferences(exp->operation.children[i], pathPatternCtx);
			}
			break;
		}
		case AL_OPERAND: {
			if (AlgebraicExpression_OperandIsReference(exp)) {
				PathPattern *pathPattern = PathPatternCtx_GetPathPattern(pathPatternCtx, exp->operand.reference);

				if (pathPattern == NULL) {
					fprintf(stderr, "Reference %s:%d is not found in path pattern context!",
			 						exp->operand.reference.name,
									exp->operand.reference.transposed);
					ASSERT(false);
				}

				exp->operand.matrix = pathPattern->m;
			}
			break;
		}
		default:
			ASSERT("Unknow algebraic expression node type" && false);
			break;
	}
}

void AlgebraicExpression_ReplaceTransposedReferences(AlgebraicExpression *ae) {
	if (ae->type == AL_OPERATION) {
		for (int i = 0; i < array_len(ae->operation.children); ++i) {
			AlgebraicExpression *child = ae->operation.children[i];
			if (child->type == AL_OPERATION) {
				if (child->operation.op == AL_EXP_TRANSPOSE) {
					AlgebraicExpression *grand_child = child->operation.children[0];
					assert(grand_child->type == AL_OPERAND && "Transpose op must have operand child");

					if (AlgebraicExpression_OperandIsReference(grand_child)) {
						assert(grand_child->operand.matrix == NULL);

						const char *src = grand_child->operand.dest;
						const char *dest = grand_child->operand.src;
						const char *label = grand_child->operand.label;
						const char *edge = grand_child->operand.edge;

						// Free transpose subtree
						AlgebraicExpression_Free(child);

						// Replace it by new reference
						AlgExpReference algexp_ref = grand_child->operand.reference;
						algexp_ref.transposed = true;
						ae->operation.children[i] = AlgebraicExpression_NewOperand(
								NULL, false, dest, src, edge, label, algexp_ref);
					}
				} else {
					AlgebraicExpression_ReplaceTransposedReferences(ae->operation.children[i]);
				}
			}
		}

	}
}
