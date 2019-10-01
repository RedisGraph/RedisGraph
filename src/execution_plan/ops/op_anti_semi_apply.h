/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"
#include "op_argument.h"

/* AntiSemiApply operation tests for the absence of a pattern
 * It starts by pulling on the left-hand side branch,
 * for each record received it pulls the right-hand side stream
 * if no data is produced it will pass onward the left-hand side record
 * otherwise the procedure repeats itself. */
typedef struct {
	OpBase op;
    Record r;           // Lefthand-side record.
    Argument *op_arg;   // Righthand-side tap.
} AntiSemiApply;

OpBase *NewAntiSemiApplyOp(void);
OpResult AntiSemiApplyInit(OpBase *opBase);
Record AntiSemiApplyConsume(OpBase *opBase);
OpResult AntiSemiApplyReset(OpBase *opBase);
void AntiSemiApplyFree(OpBase *opBase);
