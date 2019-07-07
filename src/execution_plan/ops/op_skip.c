/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "op_skip.h"

OpBase* NewSkipOp(unsigned int rec_to_skip) {
    OpSkip *skip = malloc(sizeof(OpSkip));
    skip->rec_to_skip = rec_to_skip;
    skip->skipped = 0;

    // Set our Op operations
    OpBase_Init(&skip->op);
    skip->op.name = "Skip";
    skip->op.type = OPType_SKIP;
    skip->op.consume = SkipConsume;
    skip->op.reset = SkipReset;
    skip->op.free = SkipFree;

    return (OpBase*)skip;
}

Record SkipConsume(OpBase *op) {
    OpSkip *skip = (OpSkip*)op;
    OpBase *child = skip->op.children[0];

    // As long as we're required to skip
    while(skip->skipped < skip->rec_to_skip) {
        Record discard = OpBase_Consume(child);
        
        // Depleted.
        if(!discard) return NULL;

        // Discard.
        Record_Free(discard);

        // Advance.
        skip->skipped++;
    }

    return OpBase_Consume(child);
}

OpResult SkipReset(OpBase *ctx) {
    OpSkip *skip = (OpSkip*)ctx;
    skip->skipped = 0;
    return OP_OK;
}

void SkipFree(OpBase *ctx) {

}
