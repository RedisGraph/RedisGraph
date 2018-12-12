/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_filter.h"

OpBase* NewFilterOp(FT_FilterNode *filterTree) {
    Filter *filter = malloc(sizeof(Filter));
    filter->filterTree = filterTree;

    // Set our Op operations
    OpBase_Init(&filter->op);
    filter->op.name = "Filter";
    filter->op.type = OPType_FILTER;
    filter->op.consume = FilterConsume;
    filter->op.reset = FilterReset;
    filter->op.free = FilterFree;

    return (OpBase*)filter;
}

/* FilterConsume next operation 
 * returns OP_OK when graph passes filter tree. */
OpResult FilterConsume(OpBase *opBase, Record r) {
    Filter *filter = (Filter*)opBase;
    OpBase *child = filter->op.children[0];
    int pass = FILTER_FAIL;

    while(pass != FILTER_PASS) {
        OpResult res = child->consume(child, r);
        if(res != OP_OK) return res;

        /* Pass graph through filter tree */
        pass = FilterTree_applyFilters(filter->filterTree, r);
    }

    return OP_OK;
}

/* Restart iterator */
OpResult FilterReset(OpBase *ctx) {
    return OP_OK;
}

/* Frees Filter*/
void FilterFree(OpBase *ctx) {
}
