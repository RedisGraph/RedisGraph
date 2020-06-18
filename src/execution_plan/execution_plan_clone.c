/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "execution_plan_clone.h"
#include "../query_ctx.h"
#include "../util/rax_extensions.h"

// Clones an execution plan operations, with respect to the original execution plan segment.
static OpBase *_ExecutionPlan_CloneOperations(const ExecutionPlan *orig_plan,
											  ExecutionPlan *clone_plan, const OpBase *op) {
	// If there is no op, or the op is a part of a different segment, return NULL.
	if(!op || op->plan != orig_plan) return NULL;

	// Clone the op.
	OpBase *clone = OpBase_Clone(clone_plan, op);
	// Clone the op's children and add them the cloned children array.
	for(uint i = 0; i < op->childCount; i++) {
		OpBase *cloned_child = _ExecutionPlan_CloneOperations(orig_plan, clone_plan,
															  op->children[i]);
		// Assumption: all the children are either in the same segment or on a different segment.
		if(!cloned_child) break;
		ExecutionPlan_AddOp(clone, cloned_child);
	}
	return clone;
}

// Merge cloned execution plan segments, with respect to the plan type (union or not).
static void _ExecutionPlan_MergeSegments(ExecutionPlan *plan) {
	// No need to merge segments if there aren't any.
	uint segment_count = array_len(plan->segments);
	if(segment_count == 0) return;

	if(plan->is_union) {
		// Locate the join operation.
		OpBase *join_op = ExecutionPlan_LocateOp(plan->root, OPType_JOIN);
		assert(join_op);
		// Each segment is a sub execution plan that needs to be joined.
		for(int i = 0; i < segment_count; i++) {
			ExecutionPlan *sub_plan = plan->segments[i];
			ExecutionPlan_AddOp(join_op, sub_plan->root);
		}
	} else {
		array_append(plan->segments, plan);
		segment_count = array_len(plan->segments);
		// Plan is not union, concatenate the segments.
		OpBase *connecting_op;
		// segments[0] is the first segment of the execution.
		OpBase *prev_root = plan->segments[0]->root;
		for(uint i = 1; i < segment_count; i++) {
			ExecutionPlan *current_segment = plan->segments[i];
			connecting_op = ExecutionPlan_LocateOpMatchingType(current_segment->root, PROJECT_OPS,
															   PROJECT_OP_COUNT);
			assert(connecting_op->childCount == 0);
			ExecutionPlan_AddOp(connecting_op, prev_root);
			prev_root = current_segment->root;
		}
		array_pop(plan->segments);
	}
}

/* This function clones execution plan by cloning each segment in the execution plan as a unit.
 * Each segment has its own filter tree, record mapping, query graphs and ast segment, that compose
 * a single logical execution unit, together with the segment operations.
 * The ast segment is shallow copied while all the other objects are deep cloned.
 */
ExecutionPlan *ExecutionPlan_Clone(const ExecutionPlan *template) {
	if(template == NULL) return NULL;
	// Verify that the execution plan template is not prepared yet.
	assert(template->prepared == false && "Execution plan cloning should be only on templates");
	// Allocate an empty execution plan.
	ExecutionPlan *clone = ExecutionPlan_NewEmptyExecutionPlan();

	clone->is_union = template->is_union;
	clone->record_map = raxClone(template->record_map);
	if(template->ast_segment) clone->ast_segment = AST_ShallowCopy(template->ast_segment);
	if(template->query_graph) clone->query_graph = QueryGraph_Clone(template->query_graph);
	if(template->connected_components) {
		array_clone_with_cb(clone->connected_components, template->connected_components, QueryGraph_Clone);
	}

	// The execution plan segment clone requires the specific AST segment for referenced entities.
	AST *master_ast = QueryCtx_GetAST();
	QueryCtx_SetAST(clone->ast_segment);
	// Clone each operation in the template relevant segment.
	clone->root = _ExecutionPlan_CloneOperations(template, clone, template->root);
	// After clone, restore master ast.
	QueryCtx_SetAST(master_ast);

	if(template->segments) {
		array_clone_with_cb(clone->segments, template->segments, ExecutionPlan_Clone);
	}
	// Merge the segments
	_ExecutionPlan_MergeSegments(clone);
	return clone;
}

