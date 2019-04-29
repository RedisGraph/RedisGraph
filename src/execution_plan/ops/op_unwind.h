/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_UNWIND_H
#define __OP_UNWIND_H

#include "op.h"
#include "../../arithmetic/arithmetic_expression.h"

/* OP Unwind */

typedef struct {
    OpBase op;
    AR_ExpNode **expressions;   // Array of expressions
    uint record_len;            // Length of record to create.
    uint expIdx;                // Current expression index to evaluate.
    int unwindRecIdx;           // Update record at this index.
 } OpUnwind;

/* Creates a new Unwind operation */
OpBase* NewUnwindOp(uint record_len, uint record_idx, AR_ExpNode **exprs, char *alias);

OpResult UnwindInit(OpBase *opBase);

/* UnwindConsume next operation */
Record UnwindConsume(OpBase *opBase);

/* Restart */
OpResult UnwindReset(OpBase *ctx);

/* Frees Unwind*/
void UnwindFree(OpBase *ctx);

#endif
