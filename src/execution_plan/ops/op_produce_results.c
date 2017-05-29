#include "op_produce_results.h"
#include "../../resultset/record.h"

// OpBase* NewProduceResultsOp(RedisModuleCtx *ctx, RedisModuleString *graph, QueryExpressionNode *ast) {
void NewProduceResultsOp(RedisModuleCtx *ctx, QueryExpressionNode *ast, OpBase **op) {
    *op = (OpBase *)NewProduceResults(ctx, ast);
}

ProduceResults* NewProduceResults(RedisModuleCtx *ctx, QueryExpressionNode *ast) {
    ProduceResults *produceResults = malloc(sizeof(ProduceResults));
    produceResults->ctx = ctx;
    produceResults->ast = ast;
    produceResults->resultset = NULL;
    produceResults->refreshAfterPass = 0;

    // Set our Op operations
    produceResults->op.name = "Produce Results";
    produceResults->op.type = OPType_PRODUCE_RESULTS;
    produceResults->op.next = ProduceResultsConsume;
    produceResults->op.reset = ProduceResultsReset;
    produceResults->op.free = ProduceResultsFree;
    produceResults->op.modifies = NULL;
    return produceResults;
}

/* ProduceResults next operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *opBase, Graph* graph) {
    ProduceResults *op = (ProduceResults*)opBase;

    if(op->resultset == NULL) {
        return OP_DEPLETED;
    }

    if(op->refreshAfterPass == 1) {
        op->refreshAfterPass = 0;
        return OP_REFRESH;
    }
    
    if(!op->resultset->aggregated) {
        // Append to final result set.
        Record *r = Record_FromGraph(op->ctx, op->ast, graph);
        if(ResultSet_AddRecord(op->resultset, r) == RESULTSET_FULL) {
            return OP_ERR;
        }
    }

    // Request data refresh next time consume is called.
    op->refreshAfterPass = 1;

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
        free(op);
    }
}