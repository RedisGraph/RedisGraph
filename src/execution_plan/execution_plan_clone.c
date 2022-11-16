/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "execution_plan_clone.h"
#include "../RG.h"
#include "../query_ctx.h"
#include "../util/rax_extensions.h"
#include "execution_plan_build/execution_plan_modify.h"

static ExecutionPlan *_ClonePlanInternals(const ExecutionPlan *template) {
	ExecutionPlan *clone = ExecutionPlan_NewEmptyExecutionPlan();

	clone->record_map = raxClone(template->record_map);
	if(template->ast_segment) clone->ast_segment = AST_ShallowCopy(template->ast_segment);
	if(template->query_graph) {
		QueryGraph_ResolveUnknownRelIDs(template->query_graph);
		clone->query_graph = QueryGraph_Clone(template->query_graph);
	}
	// TODO improve QueryGraph logic so that we do not need to store or clone connected_components.
	if(template->connected_components) {
		array_clone_with_cb(clone->connected_components, template->connected_components, QueryGraph_Clone);
	}

	return clone;
}

static OpBase *_CloneOpTree(OpBase *template_parent, OpBase *template_current,
							OpBase *clone_parent) {
	const ExecutionPlan *plan_segment;
	if(!template_parent || (template_current->plan != template_parent->plan)) {
		/* If this is the first operation or it was built using a different ExecutionPlan
		 * segment than its parent, clone the ExecutionPlan segment. */
		plan_segment = _ClonePlanInternals(template_current->plan);
	} else {
		// This op was built as part of the same segment as its parent, don't change ExecutionPlans.
		plan_segment = clone_parent->plan;
	}

	// Temporarily set the thread-local AST to be the one referenced by this ExecutionPlan segment.
	QueryCtx_SetAST(plan_segment->ast_segment);

	// Clone the current operation.
	OpBase *clone_current = OpBase_Clone(plan_segment, template_current);

	for(uint i = 0; i < template_current->childCount; i++) {
		// Recursively visit and clone the op's children.
		OpBase *child_op = _CloneOpTree(template_current, template_current->children[i], clone_current);
		ExecutionPlan_AddOp(clone_current, child_op);
	}

	return clone_current;
}

static ExecutionPlan *_ExecutionPlan_Clone(const ExecutionPlan *template) {
	OpBase *clone_root = _CloneOpTree(NULL, template->root, NULL);
	// The "master" execution plan is the one constructed with the root op.
	ExecutionPlan *clone = (ExecutionPlan *)clone_root->plan;
	// The root op is currently NULL; set it now.
	clone->root = clone_root;

	return clone;
}

/* This function clones the input ExecutionPlan by recursively visiting its tree of ops.
 * When an op is encountered that was constructed as part of a different ExecutionPlan segment, that segment
 * and its internal members (FilterTree, record mapping, query graphs, and AST segment) are also cloned. */
ExecutionPlan *ExecutionPlan_Clone(const ExecutionPlan *template) {
	ASSERT(template != NULL);
	// Store the original AST pointer.
	AST *master_ast = QueryCtx_GetAST();
	// Verify that the execution plan template is not prepared yet.
	ASSERT(template->prepared == false && "Execution plan cloning should be only on templates");
	ExecutionPlan *clone = _ExecutionPlan_Clone(template);
	// Restore the original AST pointer.
	QueryCtx_SetAST(master_ast);
	return clone;
}

