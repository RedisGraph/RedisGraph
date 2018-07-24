/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_filter.h"

OpBase* NewFilterOp(FT_FilterNode *filterTree, const QueryGraph *qg) {
    return (OpBase*)NewFilter(filterTree, qg);
}

Filter* NewFilter(FT_FilterNode *filterTree, const QueryGraph *qg) {
    Filter *filter = malloc(sizeof(Filter));

    /* Bind filter tree with filter nodes. */
    FilterTree_bindEntities(filterTree, qg);

    filter->filterTree = filterTree;
    filter->state = FilterUninitialized;

    // Set our Op operations
    filter->op.name = "Filter";
    filter->op.type = OPType_FILTER;
    filter->op.consume = FilterConsume;
    filter->op.reset = FilterReset;
    filter->op.free = FilterFree;
    filter->op.modifies = NULL;
    
    return filter;
}

/* FilterConsume next operation 
 * returns OP_OK when graph passes filter tree. */
OpResult FilterConsume(OpBase *opBase, QueryGraph* graph) {
    Filter *filter = (Filter*)opBase;
    
    if(filter->state == FilterUninitialized || filter->state == FilterRequestRefresh) {
        return OP_REFRESH;
    }

    /* Pass graph through filter tree */
    int pass = FilterTree_applyFilters(filter->filterTree);

    filter->state = FilterRequestRefresh;

    /* Incase graph fails to pass filter, request new data. */
    if(pass != FILTER_PASS) return OP_REFRESH;
    
    return OP_OK;
}

/* Restart iterator */
OpResult FilterReset(OpBase *ctx) {
    Filter *filter = (Filter*)ctx;
    filter->state = FilterResetted;
    return OP_OK;
}

/* Frees Filter*/
void FilterFree(OpBase *ctx) {
    Filter *filter = (Filter*)ctx;
    free(filter);
}