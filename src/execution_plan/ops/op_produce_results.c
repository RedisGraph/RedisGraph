/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_produce_results.h"
#include "../../resultset/record.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../query_executor.h"

/* Construct arithmetic expressions from return clause. */
void _BuildArithmeticExpressions(ProduceResults* op, AST_ReturnNode *return_node, QueryGraph *graph) {
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

OpBase* NewProduceResultsOp(AST_Query *ast, ResultSet *result_set, QueryGraph* graph) {
    ProduceResults *produceResults = malloc(sizeof(ProduceResults));
    produceResults->ast = ast;
    produceResults->result_set = result_set;
    produceResults->refreshAfterPass = 0;
    produceResults->return_elements = NULL;

    _BuildArithmeticExpressions(produceResults, ast->returnNode, graph);

    // Set our Op operations
    OpBase_Init(&produceResults->op);
    produceResults->op.name = "Produce Results";
    produceResults->op.type = OPType_PRODUCE_RESULTS;
    produceResults->op.consume = ProduceResultsConsume;
    produceResults->op.reset = ProduceResultsReset;
    produceResults->op.free = ProduceResultsFree;

    return (OpBase*)produceResults;
}

/* ProduceResults consume operation
 * called each time a new result record is required */
OpResult ProduceResultsConsume(OpBase *opBase, QueryGraph* graph) {
    OpResult res = OP_DEPLETED;
    ProduceResults *op = (ProduceResults*)opBase;
    if(ResultSet_Full(op->result_set)) return OP_ERR;

    if(op->op.childCount > 0) {
        OpBase *child = op->op.children[0];
        res = child->consume(child, graph);
        if(res != OP_OK) return res;
    }

    /* Append to final result set. */
    Record *r = _ProduceResultsetRecord(op);
    if(ResultSet_AddRecord(op->result_set, r) != RESULTSET_OK) return OP_ERR;

    return res;
}

/* Restart */
OpResult ProduceResultsReset(OpBase *op) {
    return OP_OK;
}

/* Frees ProduceResults */
void ProduceResultsFree(OpBase *op) {
}
