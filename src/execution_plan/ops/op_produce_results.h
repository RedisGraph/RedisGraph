#ifndef __OP_PRODUCE_RESULTS_H
#define __OP_PRODUCE_RESULTS_H

#include "op.h"
#include "../../parser/ast.h"
#include "../../redismodule.h"
#include "../../graph/graph.h"
#include "../../graph/graph.h"
#include "../../resultset/resultset.h"

/* ProduceResults
 * generates result set */

typedef struct {
    OpBase op;
    RedisModuleCtx *ctx;
    int refreshAfterPass;
    QueryExpressionNode *ast;
    ResultSet *resultset;
} ProduceResults;


/* Creates a new NodeByLabelScan operation */
void NewProduceResultsOp(RedisModuleCtx *ctx, QueryExpressionNode *ast, OpBase **op);
ProduceResults* NewProduceResults(RedisModuleCtx *ctx, QueryExpressionNode *ast);

/* ProduceResults next operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *op, Graph* graph);

/* Restart iterator */
OpResult ProduceResultsReset(OpBase *ctx);

/* Frees ProduceResults */
void ProduceResultsFree(ProduceResults *ctx);

#endif