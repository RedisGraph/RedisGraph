#include "execution_plan.h"
#include "../util/qsort.h"
#include "ops/ops.h"
#include "../query_ctx.h"
#include "../ast/ast_mock.h"

// Sort an array and remove duplicate entries.
static void _uniqueArray(const char **arr) {
#define MODIFIES_ISLT(a,b) (strcmp((*a),(*b)) > 0)
	int count = array_len(arr);
	QSORT(const char *, arr, count, MODIFIES_ISLT);
	uint unique_idx = 0;
	for(int i = 0; i < count - 1; i ++) {
		if(arr[i] != arr[i + 1]) {
			arr[unique_idx++] = arr[i];
		}
	}
	arr[unique_idx++] = arr[count - 1];
	array_trimm_len(arr, unique_idx);
}

/* Checks if parent has given child, if so returns 1
 * otherwise returns 0 */
int _OpBase_ContainsChild(const OpBase *parent, const OpBase *child) {
	for(int i = 0; i < parent->childCount; i++) {
		if(parent->children[i] == child) {
			return 1;
		}
	}
	return 0;
}

void _OpBase_AddChild(OpBase *parent, OpBase *child) {
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

/* Removes node b from a and update child parent lists
 * Assuming B is a child of A. */
void _OpBase_RemoveNode(OpBase *parent, OpBase *child) {
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

void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
	_OpBase_RemoveNode(parent, child);
}

void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp) {
	_OpBase_AddChild(parent, newOp);
}

void _ExecutionPlan_LocateOps(OpBase *root, OPType type, OpBase ***ops) {
	if(!root) return;

	if(root->type & type) *ops = array_append(*ops, root);

	for(int i = 0; i < root->childCount; i++) {
		_ExecutionPlan_LocateOps(root->children[i], type, ops);
	}
}

OpBase **ExecutionPlan_LocateOps(OpBase *root, OPType type) {
	OpBase **ops = array_new(OpBase *, 0);
	_ExecutionPlan_LocateOps(root, type, &ops);
	return ops;
}

// Introduce the new operation B between A and A's parent op.
void ExecutionPlan_PushBelow(OpBase *a, OpBase *b) {
	// /* B is a new operation. */
	// assert(!(b->parent || b->children));

	if(a->parent == NULL) {
		/* A is the root operation. */
		_OpBase_AddChild(b, a);
		return;
	}

	/* Replace A's former parent. */
	OpBase *a_former_parent = a->parent;

	/* Disconnect A from its former parent. */
	_OpBase_RemoveChild(a_former_parent, a);

	/* Add A's former parent as parent of B. */
	_OpBase_AddChild(a_former_parent, b);

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

void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b) {
	// Insert the new operation between the original and its parent.
	ExecutionPlan_PushBelow(a, b);
	// Delete the original operation.
	ExecutionPlan_RemoveOp(plan, a);
}

void ExecutionPlan_RemoveOp(ExecutionPlan *plan, OpBase *op) {
	if(op->parent == NULL) {
		// Removing execution plan root.
		assert(op->childCount == 1);
		// Assign child as new root.
		plan->root = op->children[0];
		// Remove new root's parent pointer.
		plan->root->parent = NULL;
	} else {
		// Remove op from its parent.
		OpBase *parent = op->parent;
		_OpBase_RemoveChild(op->parent, op);

		// Add each of op's children as a child of op's parent.
		for(int i = 0; i < op->childCount; i++) {
			_OpBase_AddChild(parent, op->children[i]);
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
	OpBase *parent = op->parent;
	_OpBase_RemoveChild(op->parent, op);

	op->parent = NULL;
}

OpBase *ExecutionPlan_LocateOpResolvingAlias(OpBase *root, const char *alias) {
	if(!root) return NULL;

	uint count = (root->modifies) ? array_len(root->modifies) : 0;

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

typedef enum {
	LTR,
	RTL
} LocateOp_SearchDirection;

OpBase *_ExecutionPlan_LocateOp(OpBase *root, OPType type,
								LocateOp_SearchDirection search_direction) {
	if(!root) return NULL;

	if(root->type & type) { // NOTE - this will fail if OPType is later changed to not be a bitmask.
		return root;
	}
	if(search_direction == RTL) {
		for(int i = root->childCount - 1; i >= 0; i--) {
			OpBase *op = _ExecutionPlan_LocateOp(root->children[i], type, search_direction);
			if(op) return op;
		}
	} else {
		for(int i = 0; i < root->childCount; i++) {
			OpBase *op = _ExecutionPlan_LocateOp(root->children[i], type, search_direction);
			if(op) return op;
		}
	}

	return NULL;
}

OpBase *ExecutionPlan_LocateFirstOp(OpBase *root, OPType type) {
	return _ExecutionPlan_LocateOp(root, type, LTR);
}

OpBase *ExecutionPlan_LocateLastOp(OpBase *root, OPType type) {
	return _ExecutionPlan_LocateOp(root, type, RTL);
}

OpBase *ExecutionPlan_LocateReferences(OpBase *root, const OpBase *recurse_limit,
									   rax *refs_to_resolve) {
	if(root == recurse_limit) return NULL; // Don't traverse into earlier ExecutionPlan scopes.

	int dependency_count = 0;
	OpBase *resolving_op = NULL;
	bool all_refs_resolved = false;
	for(int i = 0; i < root->childCount && !all_refs_resolved; i++) {
		// Visit each child and try to resolve references, storing a pointer to the child if successful.
		resolving_op = ExecutionPlan_LocateReferences(root->children[i], recurse_limit, refs_to_resolve);
		if(resolving_op) dependency_count ++; // Count how many children resolved references.
		all_refs_resolved = (raxSize(refs_to_resolve) == 0); // We're done when the rax is empty.
	}

	// If we've resolved all references, our work is done.
	if(all_refs_resolved) {
		/* Return the stored child if it resolved all references, or
		 * the current op if multiple children resolved references. */
		return (dependency_count == 1) ? resolving_op : root;
	}

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

	for(int i = 0; i < op->childCount; i++) {
		ExecutionPlan_BoundVariables(op->children[i], modifiers);
	}
}

void ExecutionPlan_BindPlanToOps(OpBase *root, ExecutionPlan *plan) {
	if(!root) return;
	root->plan = plan;
	for(int i = 0; i < root->childCount; i ++) {
		ExecutionPlan_BindPlanToOps(root->children[i], plan);
	}
}

static OpBase *_buildMatchBranch(ExecutionPlan *plan, const cypher_astnode_t *path) {
	AST *ast = QueryCtx_GetAST();

	rax *bound_vars = NULL;
	const char **arguments = NULL;
	if(plan->root) {
		bound_vars = raxNew();
		// Rather than cloning the record map, collect the bound variables along with their
		// parser-generated constant strings.
		ExecutionPlan_BoundVariables(plan->root, bound_vars);
		// Prepare the variables for populating the Argument ops we will build.
		arguments = ExecutionPlan_BuildArgumentModifiesArray(bound_vars);
	}

	// Initialize an ExecutionPlan that shares this plan's Record mapping.
	ExecutionPlan *match_branch_plan = ExecutionPlan_NewEmptyExecutionPlan();
	match_branch_plan->record_map = plan->record_map;

	// If we have bound variables, build an Argument op that represents them.
	if(arguments) match_branch_plan->root = NewArgumentOp(plan, arguments);

	// Build a temporary AST holding a MATCH clause.
	AST *match_branch_ast = AST_MockMatchPattern(ast, path);

	ExecutionPlan_PopulateExecutionPlan(match_branch_plan, NULL);

	AST_MockFree(match_branch_ast);
	QueryCtx_SetAST(ast); // Reset the AST.

	OpBase *branch_match_root = match_branch_plan->root;
	// NULL-set variables shared between the match_branch_plan and the overall plan.
	match_branch_plan->root = NULL;
	match_branch_plan->record_map = NULL;
	// We can't free the plan's individual QueryGraphs, as operations like label scans
	// may later try to access their entities.
	array_free(match_branch_plan->connected_components);
	match_branch_plan->connected_components = NULL;

	// Free the temporary plan.
	ExecutionPlan_Free(match_branch_plan);

	// Associate all new ops with the correct ExecutionPlan and QueryGraph.
	ExecutionPlan_BindPlanToOps(branch_match_root, plan);

	return branch_match_root;
}

/* This method reduces a filter tree into an OpBase. The method perfrom post-order traversal over the
 * filter tree, and checks if if the current subtree rooted at the visited node contains path filter or not,
 * and either reduces the root or continue traversal and reduction.
 * The three possible operations could be:
 * 1. OpFilter - In the case the subtree originated from the root does not contains any path filters.
 * 2. OpSemiApply - In case of the current root is an expression which contains a path filter.
 * 3. ApplyMultiplexer - In case the current root is an operator (OR or AND) and at least one of its children
 * has been reduced to OpSemiApply or ApplyMultiplexer.
*/
static OpBase *_ExecutionPlan_FilterTreeToOpBaseReduction(ExecutionPlan *plan,
														  FT_FilterNode *filter_root) {
	FT_FilterNode *node;
	// Case of an expression which contains path filter.
	if(filter_root->t == FT_N_EXP && FilterTree_containsFunc(filter_root, "path_filter", &node)) {
		AR_ExpNode *expression = filter_root->exp.exp;
		bool anti = false;
		if(strcasecmp(expression->op.func_name, "not") == 0) {
			anti = true;
			expression = expression->op.children[0];
		}
		OpBase *op_semi_apply = NewSemiApplyOp(plan, anti);
		const cypher_astnode_t *path = expression->op.children[0]->operand.constant.ptrval;
		ExecutionPlan_AddOp(op_semi_apply, _buildMatchBranch(plan, path));
		return op_semi_apply;
	}
	// Case of an operator (Or or And) which its subtree contains path filter
	if(filter_root->t == FT_N_COND && FilterTree_containsFunc(filter_root, "path_filter", &node)) {
		// TODO
	}
	return NewFilterOp(plan, FilterTree_Clone(filter_root));
}

void ExecutionPlan_ReduceFilterToApply(ExecutionPlan *plan, OpBase *op) {
	assert(op->type == OPType_FILTER);
	OpFilter *filter = (OpFilter *) op;
	OpBase *apply_op = _ExecutionPlan_FilterTreeToOpBaseReduction(plan, filter->filterTree);
	ExecutionPlan_ReplaceOp(plan, op, apply_op);
	if(op == plan->root) plan->root = apply_op;
	OpBase_Free(op);
}
