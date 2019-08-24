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

typedef struct
{
    OpBase op;
    AR_ExpNode *expressions; // Atirhmetic expression (evaluated as an SIArray)
    uint listIdx;            // Current list index
    int unwindRecIdx;        // Update record at this index.
    SIValue list;            // list which the unwind operation is performed on
    Record currentRecord;    // record to clone and add a value extracted from the list
    bool isStatic;           // indicates if the unwind list is static or not
} OpUnwind;

/* Creates a new Unwind operation */
OpBase *NewUnwindOp(uint record_idx, AR_ExpNode *exprs);

OpResult UnwindInit(OpBase *opBase);

/* UnwindConsume next operation */
Record UnwindConsume(OpBase *opBase);

/* Restart */
OpResult UnwindReset(OpBase *ctx);

/* Frees Unwind*/
void UnwindFree(OpBase *ctx);

#endif
