/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_UNWIND_H
#define __OP_UNWIND_H

#include "op.h"
#include "../../parser/clauses/unwind.h"
#include "../../arithmetic/arithmetic_expression.h"

/* OP Unwind */

typedef struct {
    OpBase op;
    AST_UnwindNode *unwindClause;
    AR_ExpNode **expressions;   // Array of expressions
    uint expIdx;                // Current expression index to evaluate.
 } OpUnwind;

/* Creates a new Unwind operation */
OpBase* NewUnwindOp(AST_UnwindNode *unwindClause);

/* UnwindConsume next operation */
OpResult UnwindConsume(OpBase *opBase, Record *r);

/* Restart */
OpResult UnwindReset(OpBase *ctx);

/* Frees Unwind*/
void UnwindFree(OpBase *ctx);

#endif
