/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_filter.h"

OpBase* NewFilterOp(FT_FilterNode *filterTree, const QueryGraph *qg) {
    Filter *filter = malloc(sizeof(Filter));

    /* Bind filter tree with filter nodes. */
    FilterTree_bindEntities(filterTree, qg);
    filter->filterTree = filterTree;

    // Set our Op operations
    filter->op.name = "Filter";
    filter->op.type = OPType_FILTER;
    filter->op.consume = FilterConsume;
    filter->op.reset = FilterReset;
    filter->op.free = FilterFree;
    filter->op.modifies = NULL;
    filter->op.childCount = 0;
    filter->op.children = NULL;
    filter->op.parent = NULL;
    
    return (OpBase*)filter;
}

/* FilterConsume next operation 
 * returns OP_OK when graph passes filter tree. */
OpResult FilterConsume(OpBase *opBase, QueryGraph* graph) {
    Filter *filter = (Filter*)opBase;
    OpBase *child = filter->op.children[0];
    int pass = FILTER_FAIL;

    while(pass != FILTER_PASS) {
        OpResult res = child->consume(child, graph);
        if(res != OP_OK) return res;

        /* Pass graph through filter tree */
        pass = FilterTree_applyFilters(filter->filterTree);
    }

    return OP_OK;
}

/* Restart iterator */
OpResult FilterReset(OpBase *ctx) {
    Filter *filter = (Filter*)ctx;
    return OP_OK;
}

/* Frees Filter*/
void FilterFree(OpBase *ctx) {
}
