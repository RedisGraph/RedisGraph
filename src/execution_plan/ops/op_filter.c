#include "op_filter.h"

OpBase* NewFilterOp(FT_FilterNode *filterTree) {
    return (OpBase*)NewFilter(filterTree);
}

Filter* NewFilter(FT_FilterNode *filterTree) {
    Filter *filter = malloc(sizeof(Filter));
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
OpResult FilterConsume(OpBase *opBase, Graph* graph) {
    Filter *filter = (Filter*)opBase;
    
    if(filter->state == FilterUninitialized || filter->state == FilterRequestRefresh) {
        return OP_REFRESH;
    }

    /* Pass graph through filter tree */
    int pass = applyFilters(graph, filter->filterTree);

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
    FilterTree_Free(filter->filterTree);
    free(filter);
}