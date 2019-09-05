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

const char **_ExecutionPlan_LocateReferences(OpBase *root, OpBase **op, rax *references) {
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

		if(raxFind(references, (unsigned char *)&seen_id, strlen(seen_id)) != raxNotFound) {
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

void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
	_OpBase_RemoveNode(parent, child);
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

OpBase *ExecutionPlan_LocateOp(OpBase *root, OPType type) {
	if(!root) return NULL;

	if(root->type == type) {
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

OpBase *ExecutionPlan_LocateLeaf(OpBase *root) {
	if(root->childCount == 0) return root;
	for(int i = 0; i < root->childCount; i++) {
		return ExecutionPlan_LocateLeaf(root->children[i]);
	}
}

OpBase *ExecutionPlan_LocateReferences(OpBase *root, rax *references) {
	OpBase *op = NULL;
	const char **temp = _ExecutionPlan_LocateReferences(root, &op, references);
	array_free(temp);
	return op;
}
