#include "execution_plan_modify.h"
#include "../execution_plan.h"
#include "../../RG.h"
#include "../ops/ops.h"
#include "../../query_ctx.h"
#include "../../ast/ast_mock.h"

static void _OpBase_AddChild(OpBase *parent, OpBase *child) {
	// Add child to parent
	if(parent->children == NULL) {
		parent->children = rm_malloc(sizeof(OpBase *));
	} else {
		parent->children = rm_realloc(parent->children, sizeof(OpBase *) * (parent->childCount + 1));
	}
	parent->children[parent->childCount++] = child;

	// Add parent to child
	child->parent = parent;
}

/* Remove the operation old_child from its parent and replace it
 * with the new child without reordering elements. */
static void _ExecutionPlan_ParentReplaceChild(OpBase *parent, OpBase *old_child,
											  OpBase *new_child) {
	assert(parent->childCount > 0);

	for(int i = 0; i < parent->childCount; i ++) {
		/* Scan the children array to find the op being replaced. */
		if(parent->children[i] != old_child) continue;
		/* Replace the original child with the new one. */
		parent->children[i] = new_child;
		new_child->parent = parent;
		return;
	}

	assert(false && "failed to locate the operation to be replaced");
}

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
static void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
	// Remove child from parent.
	int i = 0;
	for(; i < parent->childCount; i++) {
		if(parent->children[i] == child) break;
	}

	assert(i != parent->childCount);

	// Update child count.
	parent->childCount--;
	if(parent->childCount == 0) {
		rm_free(parent->children);
		parent->children = NULL;
	} else {
		// Shift left children.
		for(int j = i; j < parent->childCount; j++) {
			parent->children[j] = parent->children[j + 1];
		}
		parent->children = rm_realloc(parent->children, sizeof(OpBase *) * parent->childCount);
	}

	// Remove parent from child.
	child->parent = NULL;
}

inline void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp) {
	_OpBase_AddChild(parent, newOp);
}

// Introduce the new operation B between A and A's parent op.
void ExecutionPlan_PushBelow(OpBase *a, OpBase *b) {
	if(a->parent == NULL) {
		/* A is the root operation. */
		_OpBase_AddChild(b, a);
		return;
	}

	/* Disconnect A from its parent and replace it with B. */
	_ExecutionPlan_ParentReplaceChild(a->parent, a, b);

	/* Add A as a child of B. */
	_OpBase_AddChild(b, a);
}

void ExecutionPlan_NewRoot(OpBase *old_root, OpBase *new_root) {
	/* The new root should have no parent, but may have children if we've constructed
	 * a chain of traversals/scans. */
	assert(!old_root->parent && !new_root->parent);

	/* Find the deepest child of the new root operation.
	 * Currently, we can only follow the first child, since we don't call this function when
	 * introducing Cartesian Products (the only multiple-stream operation at this stage.)
	 * This may be inadequate later. */
	OpBase *tail = new_root;
	assert(tail->childCount <= 1);
	while(tail->childCount > 0) tail = tail->children[0];

	// Append the old root to the tail of the new root's chain.
	_OpBase_AddChild(tail, old_root);
}

inline void ExecutionPlan_UpdateRoot(ExecutionPlan *plan, OpBase *new_root) {
	if(plan->root) ExecutionPlan_NewRoot(plan->root, new_root);
	plan->root = new_root;
}

void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b) {
	// Insert the new operation between the original and its parent.
	ExecutionPlan_PushBelow(a, b);
	// Delete the original operation.
	ExecutionPlan_RemoveOp(plan, a);
}

void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op) {
	if(op->parent == NULL) {
		ExecutionPlan *plan = (ExecutionPlan *)op->plan;
		// Removing execution plan root.
		assert(op->childCount == 1);
		// Assign child as new root.
		plan->root = op->children[0];
		// Remove new root's parent pointer.
		plan->root->parent = NULL;
	} else {
		OpBase *parent = op->parent;
		if(op->childCount > 0) {
			// In place replacement of the op first branch instead of op.
			_ExecutionPlan_ParentReplaceChild(op->parent, op, op->children[0]);
			// Add each of op's children as a child of op's parent.
			for(int i = 1; i < op->childCount; i++) _OpBase_AddChild(parent, op->children[i]);
		} else {
			// Remove op from its parent.
			_OpBase_RemoveChild(op->parent, op);
		}
	}

	// Clear op.
	op->parent = NULL;
	rm_free(op->children);
	op->children = NULL;
	op->childCount = 0;
}

void ExecutionPlan_DetachOp(OpBase *op) {
	// Operation has no parent.
	if(op->parent == NULL) return;

	// Remove op from its parent.
	_OpBase_RemoveChild(op->parent, op);

	op->parent = NULL;
}

OpBase *ExecutionPlan_LocateOpResolvingAlias(OpBase *root, const char *alias) {
	if(!root) return NULL;

	uint count = array_len(root->modifies);

	for(uint i = 0; i < count; i++) {
		const char *resolved_alias = root->modifies[i];
		/* NOTE - if this function is later used to modify the returned operation, we should return
		 * the deepest operation that modifies the alias rather than the shallowest, as done here. */
		if(strcmp(resolved_alias, alias) == 0) return root;
	}

	for(int i = 0; i < root->childCount; i++) {
		OpBase *op = ExecutionPlan_LocateOpResolvingAlias(root->children[i], alias);
		if(op) return op;
	}

	return NULL;
}

OpBase *ExecutionPlan_LocateOpMatchingType(OpBase *root, const OPType *types, uint type_count) {
	for(int i = 0; i < type_count; i++) {
		// Return the current op if it matches any of the types we're searching for.
		if(root->type == types[i]) return root;
	}

	for(int i = 0; i < root->childCount; i++) {
		// Recursively visit children.
		OpBase *op = ExecutionPlan_LocateOpMatchingType(root->children[i], types, type_count);
		if(op) return op;
	}

	return NULL;
}

OpBase *ExecutionPlan_LocateOp(OpBase *root, OPType type) {
	if(!root) return NULL;

	const OPType type_arr[1] = {type};
	return ExecutionPlan_LocateOpMatchingType(root, type_arr, 1);
}

static OpBase *_ExecutionPlan_LocateReferences(OpBase *root, const OpBase *recurse_limit,
											   rax *refs_to_resolve) {
	if(root == recurse_limit) return NULL; // Don't traverse into earlier ExecutionPlan scopes.

	int dependency_count = 0;
	OpBase *resolving_op = NULL;
	bool all_refs_resolved = false;
	for(int i = 0; i < root->childCount && !all_refs_resolved; i++) {
		// Visit each child and try to resolve references, storing a pointer to the child if successful.
		OpBase *tmp_op = _ExecutionPlan_LocateReferences(root->children[i], recurse_limit, refs_to_resolve);
		if(tmp_op) dependency_count ++; // Count how many children resolved references.
		// If there is more than one child resolving an op, set the root as the resolver.
		resolving_op = resolving_op ? root : tmp_op;
		all_refs_resolved = (raxSize(refs_to_resolve) == 0); // We're done when the rax is empty.
	}

	// If we've resolved all references, our work is done.
	if(all_refs_resolved) return resolving_op;

	// Try to resolve references in the current operation.
	bool refs_resolved = false;
	uint modifies_count = array_len(root->modifies);
	for(uint i = 0; i < modifies_count; i++) {
		const char *ref = root->modifies[i];
		// Attempt to remove the current op's references, marking whether any removal was succesful.
		refs_resolved |= raxRemove(refs_to_resolve, (unsigned char *)ref, strlen(ref), NULL);
	}

	if(refs_resolved) resolving_op = root;
	return resolving_op;
}

OpBase *ExecutionPlan_LocateReferences(OpBase *root, const OpBase *recurse_limit,
									   rax *refs_to_resolve) {
	OpBase *op = _ExecutionPlan_LocateReferences(root, recurse_limit, refs_to_resolve);
	return op;
}

static OpBase *_ExecutionPlan_LocateReferencesExcludingOps(OpBase *root,
														   const OpBase *recurse_limit, const OPType *ops, int op_count, rax *refs_to_resolve) {
	if(root == recurse_limit) return NULL; // Don't traverse into earlier ExecutionPlan scopes.
	int dependency_count = 0;
	OpBase *resolving_op = NULL;
	bool all_refs_resolved = false;
	bool op_is_blacklisted = false;
	// Check if this op is on the blacklist.
	for(int i = 0; i < op_count; i++) {
		if(root->type == ops[i]) {
			op_is_blacklisted = true;
			break;
		}
	}

	// If an operation is blacklisted, we shouldn't recurse into its children.
	if(op_is_blacklisted == false) {
		for(int i = 0; i < root->childCount && !all_refs_resolved; i++) {
			// Visit each child and try to resolve references, storing a pointer to the child if successful.
			OpBase *tmp_op = _ExecutionPlan_LocateReferencesExcludingOps(root->children[i], recurse_limit, ops,
																		 op_count, refs_to_resolve);
			if(tmp_op) dependency_count ++; // Count how many children resolved references.
			// If there is more than one child resolving an op, set the root as the resolver.
			resolving_op = resolving_op ? root : tmp_op;
			all_refs_resolved = (raxSize(refs_to_resolve) == 0); // We're done when the rax is empty.
		}
	}

	// If we've resolved all references, our work is done.
	if(all_refs_resolved) return resolving_op;

	// Try to resolve references in the current operation.
	bool refs_resolved = false;
	uint modifies_count = array_len(root->modifies);
	for(uint i = 0; i < modifies_count; i++) {
		const char *ref = root->modifies[i];
		// Attempt to remove the current op's references, marking whether any removal was succesful.
		refs_resolved |= raxRemove(refs_to_resolve, (unsigned char *)ref, strlen(ref), NULL);
	}

	if(refs_resolved) resolving_op = root;
	return resolving_op;
}

OpBase *ExecutionPlan_LocateReferencesExcludingOps(OpBase *root, const OpBase *recurse_limit,
												   const OPType *ops, int op_count, rax *refs_to_resolve) {
	OpBase *op = _ExecutionPlan_LocateReferencesExcludingOps(root, recurse_limit, ops, op_count,
															 refs_to_resolve);
	return op;
}

static void _ExecutionPlan_CollectOpsMatchingType(OpBase *root, const OPType *types, int type_count,
												  OpBase ***ops) {
	for(int i = 0; i < type_count; i++) {
		// Check to see if the op's type matches any of the types we're searching for.
		if(root->type == types[i]) {
			*ops = array_append(*ops, root);
			break;
		}
	}

	for(int i = 0; i < root->childCount; i++) {
		// Recursively visit children.
		_ExecutionPlan_CollectOpsMatchingType(root->children[i], types, type_count, ops);
	}
}

OpBase **ExecutionPlan_CollectOpsMatchingType(OpBase *root, const OPType *types, uint type_count) {
	OpBase **ops = array_new(OpBase *, 0);
	_ExecutionPlan_CollectOpsMatchingType(root, types, type_count, &ops);
	return ops;
}

OpBase **ExecutionPlan_CollectOps(OpBase *root, OPType type) {
	OpBase **ops = array_new(OpBase *, 0);
	const OPType type_arr[1] = {type};
	_ExecutionPlan_CollectOpsMatchingType(root, type_arr, 1, &ops);
	return ops;
}

// Collect all aliases that have been resolved by the given tree of operations.
void ExecutionPlan_BoundVariables(const OpBase *op, rax *modifiers) {
	assert(op && modifiers);
	if(op->modifies) {
		uint modifies_count = array_len(op->modifies);
		for(uint i = 0; i < modifies_count; i++) {
			const char *modified = op->modifies[i];
			raxTryInsert(modifiers, (unsigned char *)modified, strlen(modified), (void *)modified, NULL);
		}
	}

	/* Project and Aggregate operations demarcate variable scopes,
	 * collect their projections but do not recurse into their children.
	 * Note that future optimizations which operate across scopes will require different logic
	 * than this for application. */
	if(op->type == OPType_PROJECT || op->type == OPType_AGGREGATE) return;

	for(int i = 0; i < op->childCount; i++) {
		ExecutionPlan_BoundVariables(op->children[i], modifiers);
	}
}

void ExecutionPlan_BindPlanToOps(ExecutionPlan *plan, OpBase *root) {
	if(!root) return;
	// If the temporary execution plan has added new QueryGraph entities,
	// migrate them to the master plan's QueryGraph.
	QueryGraph_MergeGraphs(plan->query_graph, root->plan->query_graph);

	root->plan = plan;
	for(int i = 0; i < root->childCount; i ++) {
		ExecutionPlan_BindPlanToOps(plan, root->children[i]);
	}
}

OpBase *ExecutionPlan_BuildOpsFromPath(ExecutionPlan *plan, const char **bound_vars,
									   const cypher_astnode_t *node) {
	// Initialize an ExecutionPlan that shares this plan's Record mapping.
	ExecutionPlan *match_stream_plan = ExecutionPlan_NewEmptyExecutionPlan();
	match_stream_plan->record_map = plan->record_map;

	// If we have bound variables, build an Argument op that represents them.
	if(bound_vars) match_stream_plan->root = NewArgumentOp(plan, bound_vars);

	AST *ast = QueryCtx_GetAST();
	// Build a temporary AST holding a MATCH clause.
	cypher_astnode_type_t type = cypher_astnode_type(node);

	/* The AST node we're building a mock MATCH clause for will be a path
	 * if we're converting a MERGE clause or WHERE filter, and we must build
	 * and later free a CYPHER_AST_PATTERN node to contain it.
	 * If instead we're converting an OPTIONAL MATCH, the node is itself a MATCH clause,
	 * and we will reuse its CYPHER_AST_PATTERN node rather than building a new one. */
	bool node_is_path = (type == CYPHER_AST_PATTERN_PATH || type == CYPHER_AST_NAMED_PATH);
	AST *match_stream_ast = AST_MockMatchClause(ast, (cypher_astnode_t *)node, node_is_path);

	//--------------------------------------------------------------------------
	// Build plan's query graph
	//--------------------------------------------------------------------------

	// Extract pattern from holistic query graph.
	const cypher_astnode_t **match_clauses = AST_GetClauses(match_stream_ast, CYPHER_AST_MATCH);
	ASSERT(array_len(match_clauses) == 1);
	const cypher_astnode_t *pattern = cypher_ast_match_get_pattern(match_clauses[0]);
	array_free(match_clauses);
	QueryGraph *sub_qg = QueryGraph_ExtractPatterns(plan->query_graph, &pattern, 1);
	match_stream_plan->query_graph = sub_qg;

	ExecutionPlan_PopulateExecutionPlan(match_stream_plan);

	AST_MockFree(match_stream_ast, node_is_path);
	QueryCtx_SetAST(ast); // Reset the AST.

	// Associate all new ops with the correct ExecutionPlan and QueryGraph.
	OpBase *match_stream_root = match_stream_plan->root;
	ExecutionPlan_BindPlanToOps(plan, match_stream_root);

	// NULL-set variables shared between the match_stream_plan and the overall plan.
	match_stream_plan->root = NULL;
	match_stream_plan->record_map = NULL;
	// Free the temporary plan.
	ExecutionPlan_Free(match_stream_plan);

	return match_stream_root;
}

