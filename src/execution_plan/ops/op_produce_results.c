#include "op_produce_results.h"
#include "../../resultset/record.h"

void NewProduceResultsOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, OpBase **op) {
    *op = (OpBase *)NewProduceResults(ctx, ast);
}

ProduceResults* NewProduceResults(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast) {
    ProduceResults *produceResults = malloc(sizeof(ProduceResults));
    produceResults->ctx = ctx;
    produceResults->ast = ast;
    produceResults->resultset = NULL;
    produceResults->refreshAfterPass = 0;
    produceResults->init = 0;

    // Set our Op operations
    produceResults->op.name = "Produce Results";
    produceResults->op.type = OPType_PRODUCE_RESULTS;
    produceResults->op.consume = ProduceResultsConsume;
    produceResults->op.reset = ProduceResultsReset;
    produceResults->op.free = ProduceResultsFree;
    produceResults->op.modifies = NULL;
    return produceResults;
}

/* ProduceResults next operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *opBase, Graph* graph) {
    ProduceResults *op = (ProduceResults*)opBase;

    if(!op->init) {
        op->init = 1;
        /* Result-set is freed by module.c */
        op->resultset = NewResultSet(op->ast);
        return OP_REFRESH;
    }

    if(op->refreshAfterPass == 1) {
        op->refreshAfterPass = 0;
        return OP_REFRESH;
    }
    
    /* TODO: remove condition. */
    if(!op->resultset->aggregated) {
        /* Append to final result set. */
        Record *r = Record_FromGraph(op->ctx, op->ast, graph);
        if(ResultSet_AddRecord(op->resultset, r) == RESULTSET_FULL) {
            return OP_ERR;
        }
    }

    /* Request data refresh next time consume is called. */
    op->refreshAfterPass = 1;
    return OP_OK;
}

/* Restart */
OpResult ProduceResultsReset(OpBase *op) {
    return OP_OK;
}

/* Frees ProduceResults */
void ProduceResultsFree(OpBase *op) {
    if(op != NULL) {
        free(op);
    }
}