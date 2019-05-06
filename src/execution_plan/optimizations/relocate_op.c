/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "relocate_op.h"

/* Locate the closest projection or aggregation op
 * in the execution chain of the given operation. */
static OpBase* _locateProjection(OpBase *root) {
    if(!root) return NULL;

    if(root->type == OPType_PROJECT || root->type == OPType_AGGREGATE) {
        return root;
    }

    for(int i = 0; i < root->childCount; i++) {
        OpBase *op = _locateProjection(root->children[i]);
        if(op) return op;
    }

    return NULL;
}

/* Migrate the given operation directly above the project/aggregate operation. */
void _relocateOp(ExecutionPlanSegment *plan, OPType type) {
    OpBase *op = ExecutionPlanSegment_LocateOp(plan->root, type);
    if(!op) return;
    assert(op->childCount == 1);

    // Find the first projection or aggregation in the operation's chain
    OpBase *projection_op= _locateProjection(op->children[0]);
    assert(projection_op);

    // Remove op from its current position.
    ExecutionPlanSegment_RemoveOp(plan, op);
    // Push op right above projection/aggregation
    ExecutionPlanSegment_PushBelow(projection_op, op);
}

void relocateOperations(ExecutionPlanSegment *plan) {
    assert(plan && plan->root);
    _relocateOp(plan, OPType_LIMIT);
    _relocateOp(plan, OPType_SKIP);
    _relocateOp(plan, OPType_SORT);
    _relocateOp(plan, OPType_DISTINCT);
}
