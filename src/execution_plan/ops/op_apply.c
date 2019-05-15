/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_apply.h"

OpBase* NewApplyOp(void) {
    Apply *apply = malloc(sizeof(Apply));
    apply->init = true;
    apply->lhs_record = NULL;
    apply->rhs_records = array_new(Record, 32);
    apply->rhs_idx = 0;

    // Set our Op operations
    OpBase_Init(&apply->op);
    apply->op.name = "Apply";
    apply->op.type = OPType_APPLY;
    apply->op.consume = ApplyConsume;
    apply->op.init = ApplyInit;
    apply->op.reset = ApplyReset;
    apply->op.free = ApplyFree;

    return (OpBase*)apply;
}

OpResult ApplyInit(OpBase *opBase) {
    return OP_OK;
}

Record ApplyConsume(OpBase *opBase) {
    Apply *op = (Apply*)opBase;

    assert(op->op.childCount == 2);

    Record rhs_record;

    if(op->init) {
        // On the first call to ApplyConsume, we'll read the entirety of the
        // right-hand stream and buffer its outputs.
        op->init = false;

        OpBase *rhs = op->op.children[1];
        while((rhs_record = rhs->consume(rhs))) {
            op->rhs_records = array_append(op->rhs_records, rhs_record);
        }
    }

    OpBase *lhs = op->op.children[0];
    if (op->lhs_record == NULL) {
        // No pending data from left-hand stream
        op->rhs_idx = 0;
        op->lhs_record = lhs->consume(lhs);
    }

    if (op->lhs_record == NULL) {
        // Left-hand stream has been fully depleted
        return NULL;
    }

    // Clone the left-hand record
    Record r = Record_Clone(op->lhs_record);

    rhs_record = op->rhs_records[op->rhs_idx++];

    if (op->rhs_idx == array_len(op->rhs_records)) {
        // We've joined all data from the right-hand stream with the current
        // retrieval from the left-hand stream.
        // The next call to ApplyConsume will attempt to pull new data.
        Record_Free(op->lhs_record);
        op->lhs_record = NULL;
        op->rhs_idx = 0;
    }

    Record_Merge(&r, rhs_record);

    return r;
}

OpResult ApplyReset(OpBase *opBase) {
    Apply *op = (Apply*)opBase;
    op->init = true;
    return OP_OK;
}

void ApplyFree(OpBase *opBase) {
    Apply *op = (Apply*)opBase;
    if (op->lhs_record) Record_Free(op->lhs_record);
    if (op->rhs_records) {
        uint len = array_len(op->rhs_records);
        for (uint i = 0; i < len; i ++) {
            Record_Free(op->rhs_records[i]);
        }
        array_free(op->rhs_records);
    }
}
