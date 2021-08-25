/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "runtime_execution_plan_clone.h"
#include "../../../RG.h"
#include "../../../query_ctx.h"
#include "../../../util/rax_extensions.h"
#include "runtime_execution_plan_modify.h"

static RT_ExecutionPlan *_ClonePlanInternals(const RT_ExecutionPlan *template) {
	RT_ExecutionPlan *clone = RT_ExecutionPlan_NewEmptyExecutionPlan();
	// TODO check
	ExecutionPlan *plan_desc = (ExecutionPlan *)template->plan_desc;
	clone->plan_desc = plan_desc;
	clone->record_map = raxClone(template->record_map);
	if(plan_desc->query_graph) {
		QueryGraph_ResolveUnknownRelIDs(plan_desc->query_graph);
	}

	// Temporarily set the thread-local AST to be the one referenced by this ExecutionPlan segment.
	QueryCtx_SetAST(plan_desc->ast_segment);

	return clone;
}

static RT_OpBase *_CloneOpTree(RT_OpBase *template_parent, RT_OpBase *template_current,
							RT_OpBase *clone_parent) {
	const RT_ExecutionPlan *plan_segment;
	if(!template_parent || (template_current->plan != template_parent->plan)) {
		/* If this is the first operation or it was built using a different ExecutionPlan
		 * segment than its parent, clone the ExecutionPlan segment. */
		plan_segment = _ClonePlanInternals(template_current->plan);
	} else {
		// This op was built as part of the same segment as its parent, don't change ExecutionPlans.
		plan_segment = clone_parent->plan;
	}

	// Clone the current operation.
	RT_OpBase *clone_current = RT_OpBase_Clone(plan_segment, template_current);

	for(uint i = 0; i < template_current->childCount; i++) {
		// Recursively visit and clone the op's children.
		RT_OpBase *child_op = _CloneOpTree(template_current, template_current->children[i], clone_current);
		RT_ExecutionPlan_AddOp(clone_current, child_op);
	}

	return clone_current;
}

static RT_ExecutionPlan *_ExecutionPlan_Clone(const RT_ExecutionPlan *template) {
	RT_OpBase *clone_root = _CloneOpTree(NULL, template->root, NULL);
	// The "master" execution plan is the one constructed with the root op.
	RT_ExecutionPlan *clone = (RT_ExecutionPlan *)clone_root->plan;
	// The root op is currently NULL; set it now.
	clone->root = clone_root;

	return clone;
}

/* This function clones the input ExecutionPlan by recursively visiting its tree of ops.
 * When an op is encountered that was constructed as part of a different ExecutionPlan segment, that segment
 * and its internal members (FilterTree, record mapping, query graphs, and AST segment) are also cloned. */
RT_ExecutionPlan *RT_ExecutionPlan_Clone(const RT_ExecutionPlan *template) {
	ASSERT(template != NULL);
	// Store the original AST pointer.
	AST *master_ast = QueryCtx_GetAST();
	// Verify that the execution plan template is not prepared yet.
	ASSERT(template->prepared == false && "Execution plan cloning should be only on templates");
	RT_ExecutionPlan *clone = _ExecutionPlan_Clone(template);
	// Restore the original AST pointer.
	QueryCtx_SetAST(master_ast);
	return clone;
}
