/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_results.h"
#include "../../util/arr.h"
#include "../../query_executor.h"
#include "../../arithmetic/arithmetic_expression.h"

OpBase* NewResultsOp(ResultSet *result_set, QueryGraph* graph, GraphContext *gc) {
    Results *results = malloc(sizeof(Results));
    results->result_set = result_set;
    results->gc = gc;

    // Set our Op operations
    OpBase_Init(&results->op);
    results->op.name = "Results";
    results->op.type = OPType_RESULTS;
    results->op.consume = ResultsConsume;
    results->op.reset = ResultsReset;
    results->op.free = ResultsFree;

    return (OpBase*)results;
}

/* Results consume operation
 * called each time a new result record is required */
Record ResultsConsume(OpBase *opBase) {
    Record r = NULL;
    Results *op = (Results*)opBase;

    if(op->op.childCount) {
        OpBase *child = op->op.children[0];
        r = child->consume(child);
        if(!r) return NULL;
    }

    /* Append to final result set. */
    ResultSet_AddRecord(op->result_set, op->gc, r);
    return r;
}

/* Restart */
OpResult ResultsReset(OpBase *op) {
    return OP_OK;
}

/* Frees Results */
void ResultsFree(OpBase *opBase) {
}

