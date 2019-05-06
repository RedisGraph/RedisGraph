/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_distinct.h"
#include "../../../deps/xxhash/xxhash.h"
#include "../../util/arr.h" 

OpBase* NewDistinctOp(void) {
    OpDistinct *self = malloc(sizeof(OpDistinct));
    self->trie = NewTrieMap();

    OpBase_Init(&self->op);
    self->op.name = "Distinct";
    self->op.type = OPType_DISTINCT;
    self->op.consume = DistinctConsume;
    self->op.reset = DistinctReset;
    self->op.free = DistinctFree;

    return (OpBase*)self;
}

Record DistinctConsume(OpBase *opBase) {
    OpDistinct *self = (OpDistinct*)opBase;
    OpBase *child = self->op.children[0];

    while(true) {
        Record r = child->consume(child);
        if(!r) return NULL;

        unsigned long long const hash = Record_Hash64(r);
        int is_new = TrieMap_Add(self->trie, (char *) &hash, sizeof(hash), NULL, TrieMap_DONT_CARE_REPLACE);
        if(is_new)
            return r;
        Record_Free(r);
    }
}

OpResult DistinctReset(OpBase *ctx) {
    return OP_OK;
}

void DistinctFree(OpBase *ctx) {
    OpDistinct *self = (OpDistinct*)ctx;
    TrieMap_Free(self->trie, TrieMap_NOP_CB);
}
