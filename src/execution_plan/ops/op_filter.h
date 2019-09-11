/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
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
} OpFilter;

/* Creates a new Filter operation */
OpBase *NewFilterOp(FT_FilterNode *filterTree);

/* FilterConsume next operation
 * returns NULL when depleted. */
Record FilterConsume(OpBase *opBase);

/* Restart iterator */
OpResult FilterReset(OpBase *ctx);

/* Frees Filter*/
void FilterFree(OpBase *ctx);

#endif //_FILTER_H_
