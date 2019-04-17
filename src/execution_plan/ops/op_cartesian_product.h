/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_CARTESIANPRODUCT_H__
#define __OP_CARTESIANPRODUCT_H__

#include "op.h"

/* Cartesian product AKA Join. 
 * Currently this operation is a NOP 
 * once execution plan refactoring will take place 
 * we'll implement its logic. */
 typedef struct {
     OpBase op;
     bool init;
     Record r;
 } CartesianProduct;

OpBase* NewCartesianProductOp(int record_len);
Record CartesianProductConsume(OpBase *opBase);
OpResult CartesianProductReset(OpBase *opBase);
void CartesianProductFree(OpBase *opBase);

#endif