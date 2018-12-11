/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _FILTER_H_
#define _FILTER_H_

#include "op.h"
#include "../../filter_tree/filter_tree.h"

/* Filter
 * filters graph according to where cluase */
typedef struct {
    OpBase op;
    FT_FilterNode *filterTree;
} Filter;

/* Creates a new Filter operation */
OpBase* NewFilterOp(FT_FilterNode *filterTree);

/* FilterConsume next operation 
 * returns OP_DEPLETED when */
OpResult FilterConsume(OpBase *opBase, Record r);

/* Restart iterator */
OpResult FilterReset(OpBase *ctx);

/* Frees Filter*/
void FilterFree(OpBase *ctx);

#endif //_FILTER_H_
