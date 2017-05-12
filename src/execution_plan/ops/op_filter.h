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
    QueryExpressionNode *ast;
    FT_FilterNode *filterTree;
} Filter;

/* Creates a new Filter operation */
OpBase* NewFilterOp(RedisModuleCtx *ctx, QueryExpressionNode *ast);
Filter* NewFilter(RedisModuleCtx *ctx, QueryExpressionNode *ast);

/* FilterConsume next operation 
 * returns OP_DEPLETED when */
OpResult FilterConsume(OpBase *opBase, Graph* graph);

/* Restart iterator */
OpResult FilterReset(OpBase *ctx);

/* Frees Filter*/
void FilterFree(OpBase *ctx);

#endif //_FILTER_H_