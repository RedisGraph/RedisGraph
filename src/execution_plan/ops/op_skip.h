/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#ifndef __OP_SKIP_H
#define __OP_SKIP_H

#include "op.h"
#include "../execution_plan.h"

typedef struct {
	OpBase op;
	unsigned int rec_to_skip;
	unsigned int skipped;
} OpSkip;

OpBase *NewSkipOp(const ExecutionPlan *plan, unsigned int rec_to_skip);

Record SkipConsume(OpBase *op);

OpResult SkipReset(OpBase *ctx);

void SkipFree(OpBase *ctx);

#endif
