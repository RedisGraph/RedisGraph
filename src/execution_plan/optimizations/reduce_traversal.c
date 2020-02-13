/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_traversal.h"

#include "../../util/arr.h"
#include "../../util/vector.h"
#include "../../util/strcmp.h"
#include "../ops/op_expand_into.h"
#include "../ops/op_conditional_traverse.h"
#include "../ops/op_cond_var_len_traverse.h"

static inline bool _isInSubExecutionPlan(OpBase *op) {
	return ExecutionPlan_LocateFirstOp(op, OPType_ARGUMENT) != NULL;
}

static void _removeRedundantTraversal(ExecutionPlan *plan, CondTraverse *traverse) {
	AlgebraicExpression *ae =  traverse->ae;
	if(AlgebraicExpression_OperandCount(ae) == 1 &&
	   !RG_STRCMP(AlgebraicExpression_Source(ae), AlgebraicExpression_Destination(ae))) {
		ExecutionPlan_RemoveOp(plan, (OpBase *)traverse);
		OpBase_Free((OpBase *)traverse);
	}
}

/* Inspect each traverse operation T,
 * For each T see if T's source and destination nodes
 * are already resolved, in which case replace traversal operation
 * with expand-into op. */
void reduceTraversal(ExecutionPlan *plan) {
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
			ae = traverse->ae;
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

		if(!RG_STRCMP(AlgebraicExpression_Source(ae), AlgebraicExpression_Destination(ae)) &&
		   AlgebraicExpression_OperandCount(ae) == 1 &&
		   AlgebraicExpression_DiagonalOperand(ae, 0)) continue;

		/* Search to see if dest is already resolved */
		if(!ExecutionPlan_LocateOpResolvingAlias(op->children[0],
												 AlgebraicExpression_Destination(ae))) continue;

		/* Both src and dest are already known
		 * perform expand into instaed of traverse. */
		if(op->type == OPType_CONDITIONAL_TRAVERSE) {
			CondTraverse *traverse = (CondTraverse *)op;
			const ExecutionPlan *traverse_plan = traverse->op.plan;
			OpBase *expand_into = NewExpandIntoOp(traverse_plan, traverse->graph, traverse->ae,
												  traverse->recordsCap);

			// Set traverse algebraic_expression to NULL to avoid early free.
			traverse->ae = NULL;
			ExecutionPlan_ReplaceOp(plan, (OpBase *)traverse, expand_into);
			OpBase_Free((OpBase *)traverse);
		} else {
			CondVarLenTraverse *traverse = (CondVarLenTraverse *)op;
			const ExecutionPlan *traverse_plan = traverse->op.plan;
			CondVarLenTraverseOp_ExpandInto(traverse);
			/* Conditional variable length traversal do not perform
			 * label filtering by matrix matrix multiplication
			 * it introduces conditional traverse operation in order
			 * to perform label filtering, but in case a node is already
			 * resolved this filtering is redundent and should be removed. */
			OpBase *t;
			QGNode *src = QueryGraph_GetNodeByAlias(traverse_plan->query_graph, AlgebraicExpression_Source(ae));
			if(src->label) {
				t = op->children[0];
				if(t->type == OPType_CONDITIONAL_TRAVERSE && !_isInSubExecutionPlan(op)) {
					// Queue traversal for removal.
					redundantTraversals[redundantTraversalsCount++] = (CondTraverse *)t;
				}
			}
			QGNode *dest = QueryGraph_GetNodeByAlias(traverse_plan->query_graph,
													 AlgebraicExpression_Destination(ae));
			if(dest->label) {
				t = op->parent;
				if(t->type == OPType_CONDITIONAL_TRAVERSE && !_isInSubExecutionPlan(op)) {
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

