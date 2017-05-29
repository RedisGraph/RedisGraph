#include "op_filter.h"

OpBase* NewFilterOp(RedisModuleCtx *ctx, FT_FilterNode *filterTree) {
    return NewFilter(ctx, filterTree);
}

Filter* NewFilter(RedisModuleCtx *ctx, FT_FilterNode *filterTree) {
    Filter *filter = malloc(sizeof(Filter));
    filter->ctx = ctx;
    filter->filterTree = filterTree;
    filter->refreshAfterPass = -1;

    // Set our Op operations
    filter->op.name = "Filter";
    filter->op.type = OPType_FILTER;
    filter->op.next = FilterConsume;
    filter->op.reset = FilterReset;
    filter->op.free = FilterFree;
    filter->op.modifies = NULL;
    
    return filter;
}

/* FilterConsume next operation 
 * returns OP_OK when graph passes filter tree. */
OpResult FilterConsume(OpBase *opBase, Graph* graph) {
    Filter *filter = opBase;
    
    if(filter->refreshAfterPass == -1) {
        return OP_DEPLETED;
    }
    
    if(filter->refreshAfterPass == 1) {
        filter->refreshAfterPass = 0;
        return OP_REFRESH;
    }

    // Pass graph through filter tree
    int pass = applyFilters(filter->ctx, graph, filter->filterTree);

    // Incase graph fails to pass filter, request new data.
    if(pass == FILTER_PASS) {
        filter->refreshAfterPass = 1;
        return OP_OK;
    } else { 
        return OP_REFRESH;
    }
}

/* Restart iterator */
OpResult FilterReset(OpBase *ctx) {
    Filter *filter = ctx;
    filter->refreshAfterPass = 0;
    return OP_OK;
}

/* Frees Filter*/
void FilterFree(OpBase *ctx) {
    Filter *filter = ctx;
    if(filter->filterTree != NULL) {
        FilterTree_Free(filter->filterTree);
    }
    free(filter);
}