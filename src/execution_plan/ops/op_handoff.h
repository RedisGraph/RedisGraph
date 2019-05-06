/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"

typedef struct {
    OpBase op;
    Record r;
    // AR_ExpNode **exps;
    const char **projections;
} OpHandoff;

OpBase* NewHandoffOp(const char **projections);

OpResult HandoffInit(OpBase *opBase);

Record HandoffConsume(OpBase *op);

OpResult HandoffReset(OpBase *ctx);

void HandoffFree(OpBase *ctx);
