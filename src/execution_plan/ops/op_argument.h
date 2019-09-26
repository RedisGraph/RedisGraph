/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "op.h"

/* The Argument operation simply pass on it's internal record `r` 
 * down the execution stream, once passed the operation sets `r` to NULL.
 * `r` is expected to be set by a call to ArgumentSetRecord by some other 
 * operation (Apply*) down the execution stream. */
typedef struct {
	OpBase op;
    Record r;
} Argument;

OpBase *NewArgumentOp(void);
Record ArgumentConsume(OpBase *opBase);
OpResult ArgumentReset(OpBase *opBase);
void ArgumentSetRecord(Argument *arg, Record r);
void ArgumentFree(OpBase *opBase);
