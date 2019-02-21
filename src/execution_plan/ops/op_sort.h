/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_SORT_H
#define __OP_SORT_H

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
} Sort;

/* Creates a new Sort operation */
OpBase *NewSortOp(const AST *ast);

Record SortConsume(OpBase *opBase);

/* Restart iterator */
OpResult SortReset(OpBase *ctx);

/* Frees Sort */
void SortFree(OpBase *ctx);

#endif
