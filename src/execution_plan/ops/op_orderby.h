/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_ORDERBY_H
#define __OP_ORDERBY_H

#include "op.h"
#include "../../util/heap.h"
#include "../../arithmetic/arithmetic_expression.h"

#define DIR_DESC -1
#define DIR_ASC 1

typedef struct {
    OpBase op;
    const AST* ast;
	int direction;          // Ascending / desending.
    heap_t *heap;           // Holds top n records.
    Record *buffer;         // Holds all records.
    uint limit;             // Total number of records to produce, 0 no limit.
} OrderBy;

/* Creates a new OrderBy operation */
OpBase *NewOrderByOp(const AST *ast);

/* OrderBy next operation
 * called each time a new ID is required */
Record OrderByConsume(OpBase *opBase);

/* Restart iterator */
OpResult OrderByReset(OpBase *ctx);

/* Frees OrderBy */
void OrderByFree(OpBase *ctx);

#endif
