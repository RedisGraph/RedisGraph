/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "relocate_op.h"

static void __relocateSort(ExecutionPlan *plan, OpBase *sort, OpBase *current) {
    if(current == NULL) return;

    if(current->type == OPType_PROJECT || current->type == OPType_AGGREGATE) {
        // sort is already position correctly, beneth projection.
        if(current->parent == sort) return;

        /* Move sort right beneth projection.
         * Remove sort from its current position.  */
        ExecutionPlan_RemoveOp(plan, sort);
        // TODO: move this into ExecutionPlan_RemoveOp.
        sort->parent = NULL;
        sort->children = NULL;
        sort->childCount = 0;

        // Push sort right below projection.
        ExecutionPlan_PushBelow(current, sort);

        // We're done!
        return;
    }

    for(int i = 0; i < current->childCount; i++) {
        __relocateSort(plan, sort, current->children[i]);
    }
}

static void _relocateOp(ExecutionPlan *plan, OpBase *op, OpBase *current) {
    if(current == NULL) return;

    if(current->type == OPType_PROJECT) {
        OpBase *projection = current;

        // Remove op from its current position.
        ExecutionPlan_RemoveOp(plan, op);
        // TODO: move this into ExecutionPlan_RemoveOp.
        op->parent = NULL;
        op->children = NULL;
        op->childCount = 0;

        // Projection should have a single child.
        assert(projection->childCount == 1);

        // Push op right below projection only child.
        ExecutionPlan_PushBelow(projection->children[0], op);
        
        // We're done!
        return;
    }

    if(current->type == OPType_AGGREGATE) {
        OpBase *aggregate = current;
        // op is already position correctly, beneth aggregate.
        if(aggregate->parent == op) return;

        /* Move op right beneth aggregate.
         * Remove op from its current position.  */
        ExecutionPlan_RemoveOp(plan, op);
        // TODO: move this into ExecutionPlan_RemoveOp.
        op->parent = NULL;
        op->children = NULL;
        op->childCount = 0;

        // Push op right below aggregate.
        ExecutionPlan_PushBelow(aggregate, op);

        // We're done!
        return;
    }

    if(current->type == OPType_SORT) {
        OpBase *sort = current;
        // op is already position correctly, beneth sort.
        if(sort->parent == op) return;

        /* Move op right beneth sort.
         * Remove op from its current position.  */
        ExecutionPlan_RemoveOp(plan, op);
        // TODO: move this into ExecutionPlan_RemoveOp.
        op->parent = NULL;
        op->children = NULL;
        op->childCount = 0;

        // Push op right below sort.
        ExecutionPlan_PushBelow(sort, op);

        // We're done!
        return;
    }

    for(int i = 0; i < current->childCount; i++) {
        _relocateOp(plan, op, current->children[i]);
    }
}

/* Locate first occurrence of operation of given type within execution plan.
 * Returns NULL if operation wasn't found . */
static OpBase* _locateOp(OpBase *root, OPType type) {
    if(!root) return NULL;

    if(root->type == type) {
        return root;
    }

    for(int i = 0; i < root->childCount; i++) {
        OpBase *op = _locateOp(root->children[i], type);
        if(op) return op;
    }

    return NULL;
}

void _relocateSkip(ExecutionPlan *plan) {
    OpBase *skip = _locateOp(plan->root, OPType_SKIP);
    if(!skip) return;
    
    assert(skip->childCount == 1);
    _relocateOp(plan, skip, skip->children[0]);
}

void _relocateLimit(ExecutionPlan *plan) {
    OpBase *limit = _locateOp(plan->root, OPType_LIMIT);
    if(!limit) return;
    
    assert(limit->childCount == 1);
    _relocateOp(plan, limit, limit->children[0]);
}

void _relocateSort(ExecutionPlan *plan) {
    OpBase *sort = _locateOp(plan->root, OPType_SORT);
    if(!sort) return;
    
    assert(sort->childCount == 1);
    __relocateSort(plan, sort, sort->children[0]);
}

void relocateOperations(ExecutionPlan *plan) {
    assert(plan && plan->root);

    _relocateSort(plan);
    _relocateLimit(plan);
    _relocateSkip(plan);
}