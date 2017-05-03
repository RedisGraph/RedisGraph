#include "filter.h"

OpBase* NewFilterOp(RedisModuleCtx *ctx, QueryExpressionNode *ast) {
    return NewFilter(ctx, ast);
}

Filter* NewFilter(RedisModuleCtx *ctx, QueryExpressionNode *ast) {
    Filter *filter = malloc(sizeof(Filter));
    filter->ctx = ctx;
    filter->ast = ast;
    filter->filterTree = NULL;
    filter->refreshAfterPass = 0;

    // Set our Op operations
    filter->op.name = "Filter";
    filter->op.next = FilterConsume;
    filter->op.reset = FilterReset;
    filter->op.free = FilterFree;

    return filter;
}

/* FilterConsume next operation 
 * returns OP_DEPLETED when */
OpResult FilterConsume(OpBase *opBase, Graph* graph) {
    Filter *filter = opBase;
    
    if(filter->filterTree == NULL) {
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
    filter->filterTree = BuildFiltersTree(filter->ast->whereNode->filters);
    filter->refreshAfterPass = 0;
    return OP_OK;
}

/* Frees Filter*/
void FilterFree(OpBase *ctx) {
    Filter *filter = ctx;
    FilterTree_Free(filter->filterTree);
    free(filter);
}