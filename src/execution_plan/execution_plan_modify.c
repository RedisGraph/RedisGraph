#include "execution_plan.h"
#include "../util/qsort.h"

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

	// Add parent to child. (child can validly be NULL if we're building an empty stream for Merge.)
	if(child) child->parent = parent;
}

static void _OpBase_AddChildAtPosition(OpBase *parent, OpBase *child, int idx) {
	// The child index should be valid and unoccupied.
	assert(idx < parent->childCount && parent->children[idx] == NULL);

	// Add child to parent at the specified index.
	parent->children[idx] = child;

	// Associate parent with child.
	if(child) child->parent = parent;
}

const char **_ExecutionPlan_LocateReferences(OpBase *root, OpBase **op, rax *references) {
	if(!root) return NULL;

	/* List of entities which had their ID resolved
	 * at this point of execution, should include all
	 * previously modified entities (up the execution plan). */
	const char **seen = array_new(const char *, 0);

	uint modifies_count = array_len(root->modifies);
	/* Append current op modified entities. */
	for(uint i = 0; i < modifies_count; i++) {
		seen = array_append(seen, root->modifies[i]);
		// printf("%u\n", root->modifies[i]);
	}

	/* Traverse execution plan, upwards. */
	for(int i = 0; i < root->childCount; i++) {
		const char **saw = _ExecutionPlan_LocateReferences(root->children[i], op, references);

		/* Quick return if op was located. */
		if(*op) {
			array_free(saw);
			return seen;
		}

		uint saw_count = array_len(saw);
		/* Append current op modified entities. */
		for(uint i = 0; i < saw_count; i++) {
			seen = array_append(seen, saw[i]);
		}
		array_free(saw);
	}

	// Sort the 'seen' array and remove duplicate entries.
	// This is necessary to properly intersect 'seen' with the 'references' rax.
	_uniqueArray(seen);

	uint seen_count = array_len(seen);

	/* See if all references have been resolved. */
	uint match = raxSize(references);

	for(uint i = 0; i < seen_count; i++) {
		// Too many unmatched references.
		if(match > (seen_count - i)) break;
		const char *seen_id = seen[i];

		if(raxFind(references, (unsigned char *)seen_id, strlen(seen_id)) != raxNotFound) {
			match--;
			// All references have been resolved.
			if(match == 0) {
				*op = root;
				break;
			}
		}
	}

	// if(!match) *op = root; // TODO might be necessary for post-WITH segments
	return seen;
}

/* Returns true if the given operation is allowed to have NULL children. */
static bool _DontMigrateChildren(OpBase *op) {
	if(op->type == OPType_MERGE) return true;
	return false;
}

/* Remove NULL children from an operation. */
static void _OpBase_CompactChildren(OpBase *op) {
	int deleted_count = 0;
	for(int i = 0; i < op->childCount; i ++) {
		if(op->children[i] != NULL) continue;
		deleted_count++;
		// Shift all subsequent children to the left.
		for(int j = i; j < op->childCount - 1; j++) {
			op->children[j] = op->children[j + 1];
		}
	}

	op->childCount -= deleted_count;
	if(op->childCount == 0) {
		rm_free(op->children);
		op->children = NULL;
	} else {
		op->children = rm_realloc(op->children, sizeof(OpBase *) * op->childCount);
	}
}

/* Disconnect parent from child and return child's former index
 * (which is now occupied by a NULL pointer.) */
int _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
	// Remove child from parent.
	int i = 0;
	for(; i < parent->childCount; i++) {
		if(parent->children[i] == child) break;
	}

	assert(i != parent->childCount);

	// Remove the child pointer from the children array.
	parent->children[i] = NULL;

	// Remove parent from child.
	child->parent = NULL;

	// Return the former index of the child.
	return i;
}

void ExecutionPlan_AddOp(OpBase *parent, OpBase *newOp) {
	_OpBase_AddChild(parent, newOp);
}

void _ExecutionPlan_LocateOps(OpBase *root, OPType type, OpBase ***ops) {
	if(!root) return;

	if(root->type & type)(*ops) = array_append((*ops), root);

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
	int child_idx = _OpBase_RemoveChild(a_former_parent, a);

	/* Connect B with A's former parent. */
	_OpBase_AddChildAtPosition(a_former_parent, b, child_idx);

	/* Add A as a child of B. */
	_OpBase_AddChild(b, a);
}

void ExecutionPlan_NewRoot(OpBase *old_root, OpBase *new_root) {
	/* The new root should have no parent, but may have children if we've constructed
	 * a chain of traversals/scans. */
	assert(!old_root->parent && !new_root->parent);

	// Find the deepest child of the new root operation.
	OpBase *tail = new_root;
	while(tail->childCount > 0) tail = tail->children[0];

	// Append the old root to the tail of the new root's chain.
	_OpBase_AddChild(tail, old_root);
}

void ExecutionPlan_ReplaceOp(ExecutionPlan *plan, OpBase *a, OpBase *b) {
	if(a->parent) {
		/* Replace A's former parent. */
		OpBase *a_former_parent = a->parent;

		int a_idx;
		for(a_idx = 0; a_idx < a_former_parent->childCount; a_idx ++) {
			// Replace A with B in the parent op.
			if(a_former_parent->children[a_idx] == a) {
				a_former_parent->children[a_idx] = b;
				break;
			}
		}
		assert(a_idx < a_former_parent->childCount && "Didn't find expected child operation.");
		// Add parent op to B.
		b->parent = a_former_parent;
	}

	// Add each of op's children as a child of op's parent.
	for(int i = 0; i < a->childCount; i++) {
		_OpBase_AddChild(b, a->children[i]);
	}
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
		int child_idx = _OpBase_RemoveChild(parent, op);
		if(_DontMigrateChildren(parent)) {
			// Parent op allows NULL children (currently, only true for Merge.)
			// Migrate the child of the deleted op to the just-emptied position.
			assert(op->childCount == 1 && "Encountered op with multiple children where one is expected.");
			_OpBase_AddChildAtPosition(parent, op->children[0], child_idx);
		} else {
			_OpBase_CompactChildren(parent);
			// Add each of op's children as a child of op's parent.
			for(int i = 0; i < op->childCount; i++) {
				_OpBase_AddChild(parent, op->children[i]);
			}
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
	_OpBase_CompactChildren(parent);

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

OpBase *ExecutionPlan_LocateOp(OpBase *root, OPType type) {
	if(!root) return NULL;

	if(root->type & type) { // NOTE - this will fail if OPType is later changed to not be a bitmask.
		return root;
	}

	for(int i = 0; i < root->childCount; i++) {
		OpBase *op = ExecutionPlan_LocateOp(root->children[i], type);
		if(op) return op;
	}

	return NULL;
}

void ExecutionPlan_Taps(OpBase *root, OpBase ***taps) {
	if(root == NULL) return;
	if(root->type & OP_SCAN) *taps = array_append(*taps, root);

	for(int i = 0; i < root->childCount; i++) {
		ExecutionPlan_Taps(root->children[i], taps);
	}
}

OpBase *ExecutionPlan_LocateReferences(OpBase *root, rax *references) {
	OpBase *op = NULL;
	const char **temp = _ExecutionPlan_LocateReferences(root, &op, references);
	array_free(temp);
	return op;
}

// Collect all aliases that have been resolved by the given tree of operations.
void ExecutionPlan_BoundVariables(const OpBase *op, rax *modifiers) {
	assert(op && modifiers);
	if(op->modifies) {
		uint modifies_count = array_len(op->modifies);
		for(uint i = 0; i < modifies_count; i++) {
			const char *modified = op->modifies[i];
			raxTryInsert(modifiers, (unsigned char *)modified, strlen(modified), NULL, NULL);
		}
	}

	for(int i = 0; i < op->childCount; i++) {
		ExecutionPlan_BoundVariables(op->children[i], modifiers);
	}
}

