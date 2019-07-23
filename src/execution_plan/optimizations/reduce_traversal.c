/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_traversal.h"

#include "../../util/arr.h"
#include "../../util/vector.h"
#include "../ops/op_expand_into.h"
#include "../ops/op_conditional_traverse.h"
#include "../ops/op_cond_var_len_traverse.h"

static bool _entity_resolved(OpBase *root, const char *entity) {
	int count = (root->modifies) ? Vector_Size(root->modifies) : 0;

	for(int i = 0; i < count; i++) {
		char *alias;
		Vector_Get(root->modifies, i, &alias);
		if(strcmp(entity, alias) == 0) return true;
	}

	for(int i = 0; i < root->childCount; i++) {
		OpBase *child = root->children[i];
		if(_entity_resolved(child, entity)) return true;
	}

	return false;
}

void _removeRedundantTraversal(ExecutionPlan *plan, CondTraverse *traverse) {
	AlgebraicExpression *ae =  traverse->algebraic_expression;
	if(ae->operand_count == 1 && ae->src_node == ae->dest_node) {
		ExecutionPlan_RemoveOp(plan, (OpBase *)traverse);
		OpBase_Free((OpBase *)traverse);
	}
}

/* Inspect each traverse operation T,
 * For each T see if T's source and destination nodes
 * are already resolved, in which case replace traversal operation
 * with expand-into op. */
void reduceTraversal(ExecutionPlan *plan, AST *ast) {
	OPType t = OPType_CONDITIONAL_TRAVERSE | OPType_CONDITIONAL_VAR_LEN_TRAVERSE;
	OpBase **traversals = ExecutionPlan_LocateOps(plan->root, t);
	uint traversals_count = array_len(traversals);

	/* Keep track of redundant traversals which will be removed
	 * once we'll inspect every traversal operation. */
	uint redundantTraversalsCount = 0;
	CondTraverse *redundantTraversals[traversals_count];

	for(uint i = 0; i < traversals_count; i++) {
		OpBase *op = traversals[i];
		AlgebraicExpression *ae;

		if(op->type == OPType_CONDITIONAL_TRAVERSE) {
			CondTraverse *traverse = (CondTraverse *)op;
			ae = traverse->algebraic_expression;
		} else if(op->type == OPType_CONDITIONAL_VAR_LEN_TRAVERSE) {
			CondVarLenTraverse *traverse = (CondVarLenTraverse *)op;
			ae = traverse->ae;
		} else {
			assert(false);
		}

		/* If traverse src and dest nodes are the same,
		 * number of hops is 1 and the matrix being used is a label matrix, than
		 * traverse acts as a filter which make sure the node is of a specific type
		 * e.g. MATCH (a:A)-[e:R]->(b:B) RETURN e
		 * in this case there will be a traverse operation which will
		 * filter our dest nodes (b) which aren't of type B. */

		if(ae->src_node == ae->dest_node &&
		        ae->operand_count == 1 &&
		        ae->operands[0].diagonal) continue;

		/* Search to see if dest is already resolved */
		const char *dest = ae->dest_node->alias;
		if(!_entity_resolved(op->children[0], dest)) continue;

		/* Both src and dest are already known
		 * perform expand into instaed of traverse. */
		if(op->type == OPType_CONDITIONAL_TRAVERSE) {
			CondTraverse *traverse = (CondTraverse *)op;
			OpBase *expand_into = NewExpandIntoOp(traverse->algebraic_expression, ast);

			// Set traverse algebraic_expression to NULL to avoid early free.
			traverse->algebraic_expression = NULL;
			ExecutionPlan_ReplaceOp(plan, (OpBase *)traverse, expand_into);
			OpBase_Free((OpBase *)traverse);
		} else {
			CondVarLenTraverse *traverse = (CondVarLenTraverse *)op;
			CondVarLenTraverseOp_ExpandInto(traverse);
			/* Conditional variable length traversal do not perform
			 * label filtering by matrix matrix multiplication
			 * it introduces conditional traverse operation in order
			 * to perform label filtering, but in case a node is already
			 * resolved this filtering is redundent and should be removed. */
			OpBase *t;
			if(ae->src_node->label) {
				t = op->children[0];
				if(t->type == OPType_CONDITIONAL_TRAVERSE) {
					// Queue traversal for removal.
					redundantTraversals[redundantTraversalsCount++] = (CondTraverse *)t;
				}
			}
			if(ae->dest_node->label) {
				t = op->parent;
				if(t->type == OPType_CONDITIONAL_TRAVERSE) {
					// Queue traversal for removal.
					redundantTraversals[redundantTraversalsCount++] = (CondTraverse *)t;
				}
			}
		}
	}

	// Remove redundant traversals
	for(uint i = 0; i < redundantTraversalsCount; i++)
		_removeRedundantTraversal(plan, redundantTraversals[i]);

	// Clean up.
	array_free(traversals);
}
