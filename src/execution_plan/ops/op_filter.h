#ifndef _FILTER_H_
#define _FILTER_H_

#include "op.h"
#include "../../filter_tree/filter_tree.h"

/* Filter
 * filters graph according to where cluase */
typedef struct {
    OpBase op;
    RedisModuleCtx *ctx;
    int refreshAfterPass;
    FT_FilterNode *filterTree;
} Filter;

/* Creates a new Filter operation */
OpBase* NewFilterOp(RedisModuleCtx *ctx, FT_FilterNode *filterTree);
Filter* NewFilter(RedisModuleCtx *ctx, FT_FilterNode *filterTree);

/* FilterConsume next operation 
 * returns OP_DEPLETED when */
OpResult FilterConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult FilterReset(OpBase *ctx);

/* Frees Filter*/
void FilterFree(OpBase *ctx);

#endif //_FILTER_H_