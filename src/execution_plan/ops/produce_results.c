#include "produce_results.h"

// OpBase* NewProduceResultsOp(RedisModuleCtx *ctx, RedisModuleString *graph, QueryExpressionNode *ast) {
void NewProduceResultsOp(RedisModuleCtx *ctx, RedisModuleString *graph, QueryExpressionNode *ast, OpBase **op) {
    *op = (OpBase *)NewProduceResults(ctx, graph, ast);
    // OpBase *op = NewProduceResults(ctx, graph, ast);
    // return op;
}

ProduceResults* NewProduceResults(RedisModuleCtx *ctx, RedisModuleString *graph, QueryExpressionNode *ast) {
    ProduceResults *produceResults = malloc(sizeof(ProduceResults));
    produceResults->ctx = ctx;
    produceResults->graphName = graph;
    produceResults->ast = ast;
    produceResults->resultset = NULL;

    // Set our Op operations
    produceResults->op.name = "Produce Results";
    produceResults->op.next = ProduceResultsConsume;
    produceResults->op.reset = ProduceResultsReset;
    produceResults->op.free = ProduceResultsFree;

    return produceResults;
}

/* ProduceResults next operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *opBase, Graph* graph) {
    ProduceResults *op = (ProduceResults*)opBase;

    if(op->resultset == NULL) {
        return OP_DEPLETED;
    }

    // Check if data change since last call
    if(Graph_Compare(op->graph, graph) == 0) {
        // Request new data
        return OP_REFRESH;
    } else {
        // Graph changed, update internal graph.
        // TODO: implement a faster way to see if graph been changed.
        Graph_Free(op->graph);
        op->graph = Graph_Clone(graph);
    }
    
    return OP_OK;
}

/* Restart */
OpResult ProduceResultsReset(OpBase *op) {
    ProduceResults *pr = (ProduceResults*)op;
    
    if(pr->resultset != NULL) {
        ResultSet_Free(pr->ctx, pr->resultset);
    }
    
    pr->resultset = NewResultSet(pr->ast);
    return OP_OK;
}

/* Frees ProduceResults */
void ProduceResultsFree(ProduceResults *op) {
    if(op != NULL) {
        ResultSet_Free(op->ctx, op->resultset);
        Graph_Free(op->graph);
        free(op);
    }
}