/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_produce_results.h"
#include "../../util/arr.h"
#include "../../query_executor.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase* NewProduceResultsOp(ResultSet *result_set, QueryGraph* graph) {
    ProduceResults *produceResults = malloc(sizeof(ProduceResults));
    produceResults->result_set = result_set;

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
Record ProduceResultsConsume(OpBase *opBase) {
    Record r = NULL;
    ProduceResults *op = (ProduceResults*)opBase;
    if(ResultSet_Full(op->result_set)) return NULL;

    if(op->op.childCount) {
        OpBase *child = op->op.children[0];
        r = child->consume(child);
        if(!r) return NULL;
    }

    /* Append to final result set. */
    ResultSet_AddRecord(op->result_set, r);
    return r;
}

/* Restart */
OpResult ProduceResultsReset(OpBase *op) {
    return OP_OK;
}

/* Frees ProduceResults */
void ProduceResultsFree(OpBase *opBase) {    
}

