#include "op_produce_results.h"
#include "../../resultset/record.h"
#include "../../arithmetic_expression.h"
#include "../../query_executor.h"

OpBase* NewProduceResultsOp(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, ResultSet *result_set) {
    // *op = (OpBase *)NewProduceResults(ctx, ast, result_set);
    return (OpBase*)NewProduceResults(ctx, ast, result_set);
}

ProduceResults* NewProduceResults(RedisModuleCtx *ctx, AST_QueryExpressionNode *ast, ResultSet *result_set) {
    ProduceResults *produceResults = malloc(sizeof(ProduceResults));
    produceResults->ctx = ctx;
    produceResults->ast = ast;
    produceResults->result_set = result_set;
    produceResults->refreshAfterPass = 0;
    produceResults->return_elements = NULL;
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

/* Construct arithmetic expressions from return cluase. */
void _BuildArithmeticExpressions(ProduceResults* op, AST_ReturnNode *return_node, Graph* graph) {
    op->return_elements = NewVector(AR_ExpNode*, Vector_Size(return_node->returnElements));

    for(int i = 0; i < Vector_Size(return_node->returnElements); i++) {
        AST_ReturnElementNode *ret_node;
        Vector_Get(return_node->returnElements, i, &ret_node);

        AR_ExpNode *ae = AR_EXP_BuildFromAST(ret_node->exp, graph);
        Vector_Push(op->return_elements, ae);
    }
}

Record *_ProduceResultsetRecord(ProduceResults* op) {
    Record *r = NewRecord(Vector_Size(op->return_elements));
    for(int i = 0; i < Vector_Size(op->return_elements); i++) {
        AR_ExpNode *ae;
        Vector_Get(op->return_elements, i, &ae);
        r->values[i] = AR_EXP_Evaluate(ae);
    }
    return r;
}

/* ProduceResults next operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *opBase, Graph* graph) {
    ProduceResults *op = (ProduceResults*)opBase;

    if(!op->init) {
        _BuildArithmeticExpressions(op, op->ast->returnNode, graph);
        op->init = 1;
        return OP_REFRESH;
    }

    if(op->refreshAfterPass == 1) {
        op->refreshAfterPass = 0;
        return OP_REFRESH;
    }

    /* Append to final result set. */
    Record *r = _ProduceResultsetRecord(op);
    if(ResultSet_AddRecord(op->result_set, r) == RESULTSET_FULL) {
        return OP_ERR;
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