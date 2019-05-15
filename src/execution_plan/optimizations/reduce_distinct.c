/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "reduce_filters.h"

void reduceDistinct(ExecutionPlan *plan) {
    // Look for aggregate operation.
    OpBase *aggregate = ExecutionPlan_LocateOp(plan->root, OPType_AGGREGATE);
    if(aggregate == NULL || aggregate->parent == NULL) return;

    // See if there's a distinct operation following aggregate
    if(aggregate->parent->type == OPType_DISTINCT) {
        OpBase *distinct = aggregate->parent;
        ExecutionPlan_RemoveOp(plan, distinct);
        return;
    }

    // See if there's a distinct operation right before aggregate.
    for(int i = 0; i < aggregate->childCount; i++) {
        if(aggregate->children[i]->type == OPType_DISTINCT) {
            ExecutionPlan_RemoveOp(plan, aggregate->children[i]);
            return;
        }
    }
}
