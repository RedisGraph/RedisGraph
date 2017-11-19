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
    AST_QueryExpressionNode *ast;
    Vector *return_elements; /* Vector of arithmetic expressions. */
    ResultSet *result_set;
    int init;
} ProduceResults;


/* Creates a new NodeByLabelScan operation */
// void NewProduceResultsOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, ResultSet *resultset, OpBase **op);
OpBase* NewProduceResultsOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, ResultSet *result_set);
ProduceResults* NewProduceResults(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, ResultSet *resultset);

/* ProduceResults next operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *op, Graph* graph);

/* Restart iterator */
OpResult ProduceResultsReset(OpBase *ctx);

/* Frees ProduceResults */
void ProduceResultsFree(OpBase *ctx);

#endif