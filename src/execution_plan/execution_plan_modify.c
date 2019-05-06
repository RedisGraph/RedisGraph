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
        parent->children = malloc(sizeof(OpBase *));
    } else {
        parent->children = realloc(parent->children, sizeof(OpBase *) * (parent->childCount+1));
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

    // Uppdate child count.
    parent->childCount--;
    if(parent->childCount == 0) {
        free(parent->children);
        parent->children = NULL;
    } else {
        // Shift left children.
        for(int j = i; j < parent->childCount; j++) {
            parent->children[j] = parent->children[j+1];
        }
        parent->children = realloc(parent->children, sizeof(OpBase *) * parent->childCount);
    }

    // Remove parent from child.
    child->parent = NULL;
}

uint* _ExecutionPlanSegment_LocateReferences(OpBase *root, OpBase **op, uint *references) {
    /* List of entities which had their ID resolved
     * at this point of execution, should include all
     * previously modified entities (up the execution plan). */
    uint *seen = array_new(uint*, 0);

    uint modifies_count = array_len(root->modifies);
    /* Append current op modified entities. */
    for(uint i = 0; i < modifies_count; i++) {
        seen = array_append(seen, root->modifies[i]);
    }

    // TODO consider sorting 'seen' here
     /* Traverse execution plan, upwards. */
    for(int i = 0; i < root->childCount; i++) {
        uint *saw = _ExecutionPlanSegment_LocateReferences(root->children[i], op, references);

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

    /* See if all references have been resolved. */
    uint ref_count = array_len(references);
    uint match = ref_count;
    uint seen_count = array_len(seen);

    for(uint i = 0; i < ref_count; i++) {
        int seen_id = references[i];

        int j = 0;
        for(; j < seen_count; j++) {
            if (seen_id == seen[j]) {
                /* Match! */
                break;
            }
        }

        /* no match, quick break, */
        if (j == seen_count) break;
        else match--;
    }

    if(!match) *op = root;
    return seen;
}

void _OpBase_RemoveChild(OpBase *parent, OpBase *child) {
    _OpBase_RemoveNode(parent, child);
}

void ExecutionPlanSegment_AddOp(OpBase *parent, OpBase *newOp) {
    _OpBase_AddChild(parent, newOp);
}

void ExecutionPlanSegment_PushBelow(OpBase *a, OpBase *b) {
    /* B is a new operation. */
    assert(!(b->parent || b->children));
    assert(a->parent);

    /* Replace A's former parent. */
    OpBase *a_former_parent = a->parent;

    /* Disconnect A from its former parent. */
    _OpBase_RemoveChild(a_former_parent, a);

    /* Add A's former parent as parent of B. */
    _OpBase_AddChild(a_former_parent, b);

    /* Add A as a child of B. */
    _OpBase_AddChild(b, a);
}

void ExecutionPlanSegment_ReplaceOp(ExecutionPlanSegment *plan, OpBase *a, OpBase *b) {
    // Insert the new operation between the original and its parent.
    ExecutionPlanSegment_PushBelow(a, b);
    // Delete the original operation.
    ExecutionPlanSegment_RemoveOp(plan, a);
}

void ExecutionPlanSegment_RemoveOp(ExecutionPlanSegment *plan, OpBase *op) {
    if(op->parent == NULL) {
        // Removing execution plan root.
        assert(op->childCount == 1);
        plan->root = op->children[0];
    } else {
        // Remove op from its parent.
        OpBase* parent = op->parent;
        _OpBase_RemoveChild(op->parent, op);

        // Add each of op's children as a child of op's parent.
        for(int i = 0; i < op->childCount; i++) {
            _OpBase_AddChild(parent, op->children[i]);
        }
    }

    // Clear op.
    op->parent = NULL;
    free(op->children);
    op->children = NULL;
    op->childCount = 0;
}

OpBase* ExecutionPlanSegment_LocateOp(OpBase *root, OPType type) {
    if(!root) return NULL;

    if(root->type == type) {
        return root;
    }

    for(int i = 0; i < root->childCount; i++) {
        OpBase *op = ExecutionPlanSegment_LocateOp(root->children[i], type);
        if(op) return op;
    }

    return NULL;
}

void ExecutionPlanSegment_Taps(OpBase *root, OpBase ***taps) {
    if(root == NULL) return;
    if(root->type & OP_SCAN) *taps = array_append(*taps, root);

    for(int i = 0; i < root->childCount; i++) {
        ExecutionPlanSegment_Taps(root->children[i], taps);
    }
}

OpBase* ExecutionPlanSegment_LocateReferences(OpBase *root, uint *references) {
    OpBase *op = NULL;
    uint *temp = _ExecutionPlanSegment_LocateReferences(root, &op, references);
    array_free(temp);
    return op;
}
