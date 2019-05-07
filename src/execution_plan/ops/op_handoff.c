/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_handoff.h"

// Given the expressions that are populated by a project/aggregate operation,
// create the expressions that will be interpreted by the next ExecutionPlanSegment
// AR_ExpNode** _convertExpressions(AR_ExpNode **exps) {
    // uint exp_count = array_len(exps);
    // for (uint i = 0; i < exp_count; i ++) {
    
    // }
// }

OpBase* NewHandoffOp(const char **projections) {
    OpHandoff *handoff = malloc(sizeof(OpHandoff));

    // Set our Op operations
    OpBase_Init(&handoff->op);
    handoff->op.name = "Handoff";
    handoff->op.type = OPType_HANDOFF;
    handoff->op.consume = HandoffConsume;
    handoff->op.reset = HandoffReset;
    handoff->op.free = HandoffFree;

    // TODO valid?
    handoff->op.modifies = NULL;

    handoff->r = NULL;
    handoff->projections = projections;
    handoff->produce_record = true;

    return (OpBase*)handoff;
}

Record HandoffConsume(OpBase *opBase) {
    OpHandoff *op = (OpHandoff*)opBase;

    if (op->produce_record) {
        // In a populate context
        OpBase *child = op->op.children[0];
        op->r = child->consume(child);
        op->produce_record = false;
    } else {
        op->produce_record = true;
    }

    return op->r;
}

OpResult HandoffReset(OpBase *ctx) {
    return OP_OK;
}

void HandoffFree(OpBase *ctx) {

}