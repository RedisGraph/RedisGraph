/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "filter_variable_length_edges.h"
#include "../../util/arr.h"
#include "../ops/op_filter.h"
#include "../ops/op_cond_var_len_traverse.h"
#include "../execution_plan_build/execution_plan_modify.h"

// Returns true if the given filter operates exclusively on the traversed edge of a CondVarLenTraverse op
static bool _filterTraversedEdge(FT_FilterNode *ft, const char *src, const char *edge,
								 const char *dest) {
	bool match = false;
	// Collect all modified aliases in the FilterTree.
	rax *filtered = FilterTree_CollectModified(ft);
	// Look up the edge alias in the alias map.
	match = (raxFind(filtered, (unsigned char *)edge, strlen(edge)) != raxNotFound);
	if(match) {
		/* Reject filter trees that contain either the source or destination node.
		 * This avoids false positives on TOPATH expressions that collect all aliases in a path
		 * and rejects filters between an edge property and a traversal's src/dest property,
		 * which will not be available during traversals. */
		match = ((raxFind(filtered, (unsigned char *)src, strlen(src)) == raxNotFound) ||
				 (raxFind(filtered, (unsigned char *)dest, strlen(dest)) == raxNotFound));
	}
	raxFree(filtered);
	return match;
}

static void _filterVariableLengthEdges(ExecutionPlan *plan, CondVarLenTraverse *traverse_op) {
	// Retrieve the aliases of the traversed source, destination, and edge
	const char *src = AlgebraicExpression_Source(traverse_op->ae);
	const char *edge = AlgebraicExpression_Edge(traverse_op->ae);
	const char *dest = AlgebraicExpression_Destination(traverse_op->ae);

	OpBase *parent = traverse_op->op.parent;
	while(parent && parent->type == OPType_FILTER) {
		OpBase *grandparent = parent->parent;  // Track the next op to visit in case we free parent.
		OpFilter *filter = (OpFilter *)parent;
		FT_FilterNode *ft = filter->filterTree;

		// Check if the filter is applied to the traversed edge.
		if(_filterTraversedEdge(ft, src, edge, dest)) {
			// Embed the filter tree in the variable-length traversal.
			CondVarLenTraverseOp_Filter(traverse_op, ft);
			// NULL-set the filter tree to avoid a double free.
			filter->filterTree = NULL;
			// Free the replaced operation.
			ExecutionPlan_RemoveOp(plan, (OpBase *)filter);
			OpBase_Free((OpBase *)filter);
		}
		// Advance.
		parent = grandparent;
	}
}

void filterVariableLengthEdges(ExecutionPlan *plan) {
	ASSERT(plan != NULL);

	// Collect all variable-length traversals
	const OPType types[] = {OPType_CONDITIONAL_VAR_LEN_TRAVERSE, OPType_CONDITIONAL_VAR_LEN_TRAVERSE_EXPAND_INTO};
	OpBase **var_len_traverse_ops = ExecutionPlan_CollectOpsMatchingType(plan->root, types, 2);

	uint count = array_len(var_len_traverse_ops);
	for(uint i = 0; i < count; i ++) {
		_filterVariableLengthEdges(plan, (CondVarLenTraverse *)var_len_traverse_ops[i]);
	}

	array_free(var_len_traverse_ops);
}

