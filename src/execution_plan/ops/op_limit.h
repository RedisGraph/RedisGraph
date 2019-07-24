/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#ifndef _OP_LIMIT_H_
#define _OP_LIMIT_H_

#include "op.h"

typedef struct {
	OpBase op;
	unsigned int limit;     // Max number of records to consume.
	unsigned int consumed;  // Number of records consumed so far.
} OpLimit;

OpBase *NewLimitOp(unsigned int limit);

Record LimitConsume(OpBase *op);

OpResult LimitReset(OpBase *ctx);

void LimitFree(OpBase *ctx);

#endif
