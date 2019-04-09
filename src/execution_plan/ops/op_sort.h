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
    AR_ExpNode **expressions;   // Expression to sort by.
    heap_t *heap;               // Holds top n records.
    Record *buffer;             // Holds all records.
    unsigned int offset;        // Offset into projected order expressions within a record.
    unsigned int limit;         // Total number of records to produce, 0 no limit.
    int direction;              // Ascending / desending.
} OpSort;

/* Creates a new Sort operation */
OpBase* NewSortOp(const cypher_astnode_t *order_clause, unsigned int limit);

Record SortConsume(OpBase *opBase);

OpResult SortInit(OpBase *opBase);

/* Restart iterator */
OpResult SortReset(OpBase *ctx);

/* Frees Sort */
void SortFree(OpBase *ctx);

#endif
