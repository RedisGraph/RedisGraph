/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include "op_merge.h"

#include "../../stores/store.h"
#include "op_merge.h"
#include <assert.h>

OpBase* NewMergeOp(GraphContext *gc, QueryGraph *qg, ResultSet *result_set) {
    OpMerge *op_merge = malloc(sizeof(OpMerge));

    op_merge->gc = gc;
    op_merge->qg = qg;
    op_merge->result_set = result_set;
    op_merge->matched = false;

    // Set our Op operations
    OpBase_Init(&op_merge->op);
    op_merge->op.name = "Merge";
    op_merge->op.type = OPType_MERGE;
    op_merge->op.consume = OpMergeConsume;
    op_merge->op.reset = OpMergeReset;
    op_merge->op.free = OpMergeFree;

    return (OpBase*)op_merge;
}

OpResult OpMergeConsume(OpBase *opBase, Record *r) {
    OpMerge *op = (OpMerge*)opBase;

    OpBase *child = op->op.children[0];
    OpResult res = child->consume(child, r);
    if(res == OP_OK) {
        /* If we're here that means pattern was matched! 
        * in that case there's no need to create any graph entity,
        * we can simply return. */
        op->matched = true;
        return OP_DEPLETED;
    }
    return res;
}

OpResult OpMergeReset(OpBase *ctx) {
    // Merge doesn't modify anything.
    return OP_OK;
}

void _CreateEntities(OpMerge *op) {
    // Commit query graph and set resultset statistics.
    op->result_set->stats = CommitGraph(op->gc, op->qg);
}

void OpMergeFree(OpBase *ctx) {
    OpMerge *op = (OpMerge*)ctx;
    
    if(!op->matched) {
        /* Pattern was not matched, 
         * create every single entity within the pattern. */
        _CreateEntities(op);
    }
}
