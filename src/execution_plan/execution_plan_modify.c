#include "execution_plan.h"

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
	/* B is a new operation. */
	assert(!(b->parent || b->children));

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

