/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <assert.h>
#include "op_unwind.h"
#include "../../util/arr.h"
#include "../../arithmetic/arithmetic_expression.h"

// TODO alias unused, remove
OpBase* NewUnwindOp(uint record_len, uint record_idx, AR_ExpNode **exps, const char *alias) {
    OpUnwind *unwind = malloc(sizeof(OpUnwind));
    unwind->expIdx = 0;
    unwind->expressions = exps;

    // Set our Op operations
    OpBase_Init(&unwind->op);
    unwind->op.name = "Unwind";
    unwind->op.type = OPType_UNWIND;
    unwind->op.consume = UnwindConsume;
    unwind->op.init = UnwindInit;
    unwind->op.reset = UnwindReset;
    unwind->op.free = UnwindFree;

    // Handle introduced entity
    unwind->op.modifies = array_new(uint, 1);
    unwind->op.modifies = array_append(unwind->op.modifies, record_idx);
    unwind->unwindRecIdx = record_idx;

    return (OpBase*)unwind;
}

OpResult UnwindInit(OpBase *opBase) {
    OpUnwind *op = (OpUnwind*)opBase;
    return OP_OK;
}

Record UnwindConsume(OpBase *opBase) {
    OpUnwind *op = (OpUnwind*)opBase;

    // Evaluated and returned all expressions.
    if(op->expIdx == array_len(op->expressions)) return NULL;

    AR_ExpNode *exp = op->expressions[op->expIdx];
    Record r = Record_New(op->record_len);
    SIValue v = AR_EXP_Evaluate(exp, r);
    Record_AddScalar(r, op->unwindRecIdx, v);
    op->expIdx++;

    return r;
}

OpResult UnwindReset(OpBase *ctx) {
    OpUnwind *unwind = (OpUnwind*)ctx;
    unwind->expIdx = 0;
    return OP_OK;
}

void UnwindFree(OpBase *ctx) {
    OpUnwind *unwind = (OpUnwind*)ctx;
    
    if(unwind->expressions) {
        uint expCount = array_len(unwind->expressions);
        for(uint i = 0; i < expCount; i++) AR_EXP_Free(unwind->expressions[i]);
        array_free(unwind->expressions);
    }
}
